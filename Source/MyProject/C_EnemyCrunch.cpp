#include "C_EnemyCrunch.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "C_MyGameMode.h"


AC_EnemyCrunch::AC_EnemyCrunch()
{
	PrimaryActorTick.bCanEverTick = true;

	// [이동 설정] (Wraith보다 느리고 묵직하게 설정)
	bUseControllerRotationYaw = false; 
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
	GetCharacterMovement()->MaxWalkSpeed = 450.0f; // Crunch 450.0f

    AttackRange = 150.0f;
}

void AC_EnemyCrunch::BeginPlay()
{
	Super::BeginPlay(); // AC_EnemyBase의 BeginPlay(체력 초기화) 실행

	// [5초 대기 로직 유지]
	bIsWaiting = true;
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyCrunch::OnWaitFinished, 5.0f, false);
}

void AC_EnemyCrunch::OnWaitFinished()
{
	bIsWaiting = false;
}

void AC_EnemyCrunch::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 1. 상태 체크
    if (CurrentHealth <= 0.0f || bIsWaiting) return;

    // 2. 필수 객체 확인
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC) return;

    // [Step 1] 산개/우회 로직 업데이트 (부모 클래스 기능)
    UpdateSpreadAngleIfBlocked(DeltaTime);
    FVector FinalTargetLocation = GetSpreadTargetLocation(PlayerChar);
    float DistanceToPlayer = GetDistanceTo(PlayerChar);

    // [Step 2] 회전 로직 (행동과 분리)
    // 근접 캐릭터인 Crunch도 공격 전/중/후에 플레이어를 자연스럽게 바라봐야 합니다.
    if (DistanceToPlayer <= AttackRange * 1.5f)
    {
        FVector Direction = PlayerChar->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f;
        FRotator TargetRot = Direction.Rotation();
        FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f);
        SetActorRotation(NewRot);
    }

    // [Step 3] 행동 결정
    if (DistanceToPlayer <= AttackRange)
    {
        // 사거리 안이라면 이동 중지
        if (AIC) AIC->StopMovement();

        // 공격 중이 아닐 때만 공격 실행
        if (!bIsAttacking)
        {
            Attack();
        }
    }
    else
    {
        // 사거리 밖이고 공격 중이 아닐 때만 산개 목적지로 이동
        if (!bIsAttacking)
        {
            AIC->MoveToLocation(FinalTargetLocation);
        }
    }
}

void AC_EnemyCrunch::Attack()
{
    if (bIsAttacking || (!AttackMontage_1 && ComboStage == 0)) 
        return;

    ComboStage = FMath::Clamp(ComboStage + 1, 1, 3);
    
    UAnimMontage* TargetMontage = nullptr;
    switch (ComboStage)
    {
    case 1: TargetMontage = AttackMontage_1; break;
    case 2: TargetMontage = AttackMontage_2; break;
    case 3: TargetMontage = AttackMontage_3; break;
    default: ResetCombo(); return;
    }
    
    if (!TargetMontage) 
    {
        ResetCombo();
        return;
    }

    bIsAttacking = true;
    
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC) AIC->StopMovement();

    PlayAnimMontage(TargetMontage);

    FOnMontageEnded MontageEndedDelegate;
    MontageEndedDelegate.BindUObject(this, &AC_EnemyCrunch::ComboWindowCheck);
    GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(MontageEndedDelegate, TargetMontage);
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (PlayerChar)
    {
        // 내 공격 범위(150) + 약간의 여유(100) 안에 플레이어가 있으면 무조건 맞음
        float Distance = GetDistanceTo(PlayerChar);
        
        if (Distance <= (MeleeRadius + 100.0f)) 
        {
            UGameplayStatics::ApplyDamage(
                PlayerChar,       // 대상
                MeleeDamage,      // 대미지
                GetController(),  // 가해자 컨트롤러
                this,             // 가해자 액터
                UDamageType::StaticClass()
            );
            
            // UE_LOG(LogTemp, Warning, TEXT("Direct Hit! Player Damaged."));
            if (HitEffect)
            {
                FVector HitLocation = PlayerChar->GetActorLocation();
                UGameplayStatics::SpawnEmitterAtLocation(this, HitEffect, HitLocation, FRotator::ZeroRotator, true);
            }
        }
    }
}

// ----------------------------------------------------
// AI가 다음 콤보를 진행할지 결정하는 로직 (Anim Montages End Delegate로 사용)
void AC_EnemyCrunch::ComboWindowCheck(UAnimMontage* Montage, bool bInterrupted)
{
    bIsAttacking = false;
    
    // 만약 3타까지 다 했다면 콤보를 초기화
    if (ComboStage >= 3)
    {
        ResetCombo();
    }
}

void AC_EnemyCrunch::ResetCombo()
{
    ComboStage = 0;
    bIsAttacking = false;
    // 필요한 다른 상태 초기화
}

void AC_EnemyCrunch::OnAttackFinished()
{
	bIsAttacking = false;
}

void AC_EnemyCrunch::SetDamageActive(bool bActive)
{
    bIsDamageActive = bActive;
}



float AC_EnemyCrunch::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if(bIsDead)
    {
        return 0.0f; // 이미 죽은 적은 데미지 무시
    }

    if (DamageCauser) LastAttacker = DamageCauser;

    // 부모의 TakeDamage 호출 (여기서 체력 깎고 0이면 Die 호출)
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    return ActualDamage;
}


// C_EnemyCrunch.cpp (Die 함수 추가)

void AC_EnemyCrunch::Die()
{
    // 1. 상태 및 AI 정지 (유지)
    bIsAttacking = false;
    bIsWaiting = true;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
        AIC->UnPossess(); // [주의] 애니메이션이 재생되게 하려면 이 코드를 주석 처리하고 SetLifeSpan을 사용합니다.
    }

    // 2. 충돌 끄기
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    AC_MyGameMode* GM = Cast<AC_MyGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->OnEnemyKilled();
        UE_LOG(LogTemp, Warning, TEXT("[CRUNCH] Sent Die Notification to GameMode!")); // 확인용 로그
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[CRUNCH] Failed to find GameMode!"));
    }
    
    // 3. 사망 애니메이션 방향 계산 및 재생
    if (DeathMontage)
    {
        // DeathFor와 DeathBack 섹션 이름 정확히 분리 (DeathBack이 없으면 DeathFor로 통일)
        FName SectionName = FName("DeathFor"); 

        // [디버그]
        UE_LOG(LogTemp, Warning, TEXT("[CRUNCH DEATH] Playing Montage Section: %s"), *SectionName.ToString());
        
        float DeathAnimDuration = PlayAnimMontage(DeathMontage);
        
        if (DeathAnimDuration > 0.0f)
        {
            if (GetMesh()->GetAnimInstance())
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, DeathMontage);
			}
            
            // 4. 애니메이션 재생 시간 + 여유 시간 후 제거되도록 설정
            SetLifeSpan(DeathAnimDuration- 0.5f);
        }
    }
    else
    {
        // 몽타주가 없으면 바로 제거
        SetLifeSpan(0.0f);
    }
}