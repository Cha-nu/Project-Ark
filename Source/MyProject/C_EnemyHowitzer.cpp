#include "C_EnemyHowitzer.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "C_MyGameMode.h"

AC_EnemyHowitzer::AC_EnemyHowitzer()
{
	 PrimaryActorTick.bCanEverTick = true;

	 // [이동 설정]
	 bUseControllerRotationYaw = false; 
	 GetCharacterMovement()->bOrientRotationToMovement = true; 
	 GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f); // 회전 속도 약간 느리게
	 GetCharacterMovement()->MaxWalkSpeed = 350.0f;
}

void AC_EnemyHowitzer::BeginPlay()
{
	 Super::BeginPlay(); // AC_EnemyBase의 BeginPlay(체력 초기화) 실행됨

	 // [대기 로직 유지]
	 bIsWaiting = true;
	 GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyHowitzer::OnWaitFinished, 5.0f, false);
}

void AC_EnemyHowitzer::OnWaitFinished()
{
	 bIsWaiting = false;
}

void AC_EnemyHowitzer::Tick(float DeltaTime)
{
	 Super::Tick(DeltaTime);

	 // 죽었거나 대기 중이면 리턴
	 if (CurrentHealth <= 0.0f || bIsWaiting) return;

	 ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	 if (!PlayerChar) return;

	 float Distance = GetDistanceTo(PlayerChar);
	 AAIController* AIC = Cast<AAIController>(GetController());
	
	 if (Distance <= AttackRange) 
	 {
	 	 // [상황 A: 사거리 이내] -> 공격
	 	 if (AIC) AIC->StopMovement();

	 	 // 플레이어 방향으로 부드럽게 회전
	 	 FVector Direction = PlayerChar->GetActorLocation() - GetActorLocation();
	 	 Direction.Z = 0.0f;
	 	 FRotator TargetRot = Direction.Rotation();
	 	 FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f);
	 	 SetActorRotation(NewRot);

	 	 Attack();
	 }
	 else 
	 {
	 	 // [상황 B: 사거리 밖] -> 추격
	 	 if (!bIsAttacking)
	 	 {
	 	 	 if (AIC) AIC->MoveToActor(PlayerChar);
	 	 }
	 }
}

void AC_EnemyHowitzer::Attack()
{
    if (bIsAttacking || !AttackMontage) return;

    // ProjectileClass가 있어야만 발사 로직을 진행합니다.
    if (ProjectileClass) 
    {
        // 1. 발사 위치: 총구(Muzzle) 소켓에서 가져옴
        FVector MuzzleLocation = GetMesh()->GetSocketLocation(MuzzleSocketName);

        // 2. 목표물 확인 (플레이어)
        ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerChar)
        {
            // 목표 지점은 플레이어의 위치보다 살짝 위 (50cm)로 설정
            FVector TargetLocation = PlayerChar->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
            FVector Direction = TargetLocation - MuzzleLocation;
            FRotator MuzzleRotation = Direction.Rotation();

            // 총구 위치에서 총구 방향으로 50cm만큼 떨어진 곳에 생성
            FVector SpawnLocation = MuzzleLocation + (MuzzleRotation.Vector() * 50.0f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // 유탄(ProjectileClass) 생성 및 발사
            GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, MuzzleRotation, SpawnParams);
        }
    }
    
    bIsAttacking = true;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC) AIC->StopMovement();

    // 몽타주 재생
    float Duration = PlayAnimMontage(AttackMontage);
    float WaitTime = (Duration > 0.0f) ? Duration : 1.0f;

    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyHowitzer::OnAttackFinished, WaitTime, false);
}

void AC_EnemyHowitzer::OnAttackFinished()
{
    bIsAttacking = false;
}

float AC_EnemyHowitzer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // [1] 마지막 공격자 저장 (사망 모션 방향 계산용)
    if (DamageCauser)
    {
        LastAttacker = DamageCauser;
    }

    // [2] 부모 클래스(Base)의 체력 감소 로직 실행
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // [3] 이미 사망했다면 피격 모션 재생 없이 종료
    if (CurrentHealth <= 0.0f) 
    {
        return ActualDamage;
    }

    // [4] 피격 모션 처리 (Wraith와 동일)
    if (HitMontage && DamageCauser)
    {
        FVector AttackerLoc = DamageCauser->GetActorLocation();
        FVector Direction = (AttackerLoc - GetActorLocation()).GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), Direction);
        FVector CrossProduct = FVector::CrossProduct(GetActorForwardVector(), Direction);

        FName SectionName = FName("Front");

        // 피격 방향 판정 로직은 Wraith와 동일하게 사용
        if (DotProduct >= 0.5f)
        {
            SectionName = FName("Front");
        }
        else if (DotProduct <= -0.5f)
        {
            SectionName = FName("Back");
        }
        else 
        {
            SectionName = (CrossProduct.Z > 0.0f) ? FName("Right") : FName("Left");
        }

        // 몽타주 재생 및 섹션 점프
        PlayAnimMontage(HitMontage);
        if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, HitMontage);
		}
    }

    return ActualDamage;
}

// Die 함수를 AC_EnemyBase에서 오버라이드하여 구현
void AC_EnemyHowitzer::Die()
{
    // [1] 상태 및 AI 정지
    bIsAttacking = false;
    bIsWaiting = true;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
        AIC->UnPossess(); 
    }

    // [2] 충돌 끄기 (AC_EnemyBase는 GetCapsuleComponent()를 직접 접근 못함. ACharacter를 통해 가져옴)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

    // [3] 사망 애니메이션 방향 계산 및 재생
    if (DeathMontage && LastAttacker)
    {
        FVector AttackerLoc = LastAttacker->GetActorLocation();
        FVector Direction = (AttackerLoc - GetActorLocation()).GetSafeNormal();
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), Direction);
        
        FName SectionName = (DotProduct >= 0.0f) ? FName("DeathFor") : FName("DeathFor"); 

        float DeathAnimDuration = PlayAnimMontage(DeathMontage);
        
        if (DeathAnimDuration > 0.0f)
        {
            if (GetMesh()->GetAnimInstance())
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, DeathMontage);
			}
            
            // 애니메이션 재생 후 Lifespan 설정 (애니메이션 길이만큼 시체 유지)
            SetLifeSpan(DeathAnimDuration); // 애니메이션 지속시간 + 2초 여유
        }
        else
        {
            SetLifeSpan(0.0f); // 몽타주 없으면 바로 삭제
        }
    }
    else
    {
        // 몽타주가 없거나 공격자가 없으면 바로 삭제 예약 (0.0f는 다음 프레임에 제거)
        SetLifeSpan(0.0f);
    }
}