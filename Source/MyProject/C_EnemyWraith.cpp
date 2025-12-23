#include "C_EnemyWraith.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "C_MyGameMode.h"

AC_EnemyWraith::AC_EnemyWraith()
{
	PrimaryActorTick.bCanEverTick = true;

	// [이동 설정]
	bUseControllerRotationYaw = false; 
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
	GetCharacterMovement()->MaxWalkSpeed = 600.0f; 
}

void AC_EnemyWraith::BeginPlay()
{
	Super::BeginPlay(); // Base의 BeginPlay(체력 초기화) 실행됨

	bIsWaiting = true;
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyWraith::OnWaitFinished, 5.0f, false);
}

void AC_EnemyWraith::OnWaitFinished()
{
	bIsWaiting = false;
}

void AC_EnemyWraith::Tick(float DeltaTime)
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
		if (AIC) AIC->StopMovement();

		// 회전
		FVector Direction = PlayerChar->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
		FRotator TargetRot = Direction.Rotation();
		FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f);
		SetActorRotation(NewRot);

		Attack();
	}
	else 
	{
		if (!bIsAttacking)
		{
			if (AIC) AIC->MoveToActor(PlayerChar);
		}
	}
}

void AC_EnemyWraith::Attack()
{
	if (bIsAttacking || !AttackMontage) return;

	if (ProjectileClass)
    {
        // 1. 발사 위치: 총구(Muzzle) 소켓
        FVector MuzzleLocation = GetMesh()->GetSocketLocation(MuzzleSocketName);

        // 2. 목표물 확인 (플레이어)
        ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerChar)
        {
            FVector TargetLocation = PlayerChar->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
            FVector Direction = TargetLocation - MuzzleLocation;
            FRotator MuzzleRotation = Direction.Rotation();

		//총알이 나가는 방향(Vector)으로 50cm만큼 더한 위치를 구함
		FVector SpawnLocation = MuzzleLocation + (MuzzleRotation.Vector() * 50.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		//겹쳐도 무조건 생성
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, MuzzleRotation, SpawnParams);
				}
			}

	bIsAttacking = true;

	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC) AIC->StopMovement();

	float Duration = PlayAnimMontage(AttackMontage);
	float WaitTime = (Duration > 0.0f) ? Duration : 1.0f;

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyWraith::OnAttackFinished, WaitTime, false);
}

void AC_EnemyWraith::OnAttackFinished()
{
	bIsAttacking = false;
}

float AC_EnemyWraith::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // [1] 죽을 때 방향 계산을 위해, 마지막으로 때린 사람을 기억해둠
    if (DamageCauser)
    {
        LastAttacker = DamageCauser;
    }

    // [2] 부모 클래스(Base)의 로직 실행 (체력 감소, 로그 출력, Die() 호출 등)
    // 여기서 만약 체력이 0이 되면 Base 내부에서 Die()가 호출됩니다.
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // [3] 이미 죽었다면 '피격 모션'을 재생하지 않고 리턴
    if (CurrentHealth <= 0.0f) 
    {
        return ActualDamage;
    }

    UE_LOG(LogTemp, Error, TEXT("[DEBUG] HitMontage: %s, DamageCauser: %s"), 
    HitMontage ? *HitMontage->GetName() : TEXT("NULL"), 
    DamageCauser ? *DamageCauser->GetName() : TEXT("NULL"));
    // [4] 살아있고, 피격 몽타주가 있다면 방향 계산 후 재생
    if (HitMontage && DamageCauser)
    {
        // A. 나(Wraith)와 때린 놈(Attacker)의 위치 벡터 계산
        FVector AttackerLoc = DamageCauser->GetActorLocation();
        FVector MyLoc = GetActorLocation();
        
        // 방향 벡터 (나 -> 적)
        FVector Direction = (AttackerLoc - MyLoc).GetSafeNormal();

        // B. 내적(Dot Product)으로 앞/뒤 판별
        // 내 정면(Forward)과 적 방향(Direction)이 얼마나 일치하는가?
        // 1.0 = 완전 정면, -1.0 = 완전 후면
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), Direction);

        // C. 외적(Cross Product)으로 좌/우 판별
        // Z값이 양수면 오른쪽, 음수면 왼쪽
        FVector CrossProduct = FVector::CrossProduct(GetActorForwardVector(), Direction);

        // D. 섹션 이름 결정 (기본값 Front)
        FName SectionName = FName("Front");

        if (DotProduct >= 0.5f) // 정면 (약 60도 범위)
        {
            SectionName = FName("Front");
        }
        else if (DotProduct <= -0.5f) // 후면
        {
            SectionName = FName("Back");
        }
        else // 측면 (내적이 -0.5 ~ 0.5 사이일 때)
        {
            if (CrossProduct.Z > 0.0f)
            {
                SectionName = FName("Right");
            }
            else
            {
                SectionName = FName("Left");
            }
        }

        // E. 몽타주 재생 및 섹션 점프
        PlayAnimMontage(HitMontage);
        UE_LOG(LogTemp, Warning, TEXT("PlayAnimMontage(HitMontage) Called! Section: %s"), *SectionName.ToString());
        if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, HitMontage);
		}
    }

    return ActualDamage;
}
// [선택] 사망 처리 오버라이드
void AC_EnemyWraith::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("Die Called! Die Executed!"));
    // 1. 더 이상 공격/대기 안 함
    bIsAttacking = false;
    bIsWaiting = true;

    // 2. AI 정지
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
        AIC->UnPossess(); 
    }

    // 3. 충돌 끄기 (캡슐만 끄고 메시는 놔둬야 시체가 보임)
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

    // 4. 사망 애니메이션 방향 계산 및 재생
    if (DeathMontage && LastAttacker)
    {
        FVector AttackerLoc = LastAttacker->GetActorLocation();
        FVector MyLoc = GetActorLocation();
        FVector Direction = (AttackerLoc - MyLoc).GetSafeNormal();
        
        // 내적 계산 (1: 정면, -1: 후면)
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), Direction);
        
        FName SectionName = FName("DeathBack"); // 기본값

        // [논리]
        // 앞에서 맞음(Dot > 0) -> 충격으로 뒤로 넘어짐 
        // 뒤에서 맞음(Dot < 0) -> 충격으로 앞으로 넘어짐 
        UE_LOG(LogTemp, Error, TEXT("[DEBUG] DotProduct: %.2f"), DotProduct);
        if (DotProduct >= 0.0f)
        {
            SectionName = FName("DeathBack");
        }
        else
        {
            SectionName = FName("DeathFor");
        }

        // 몽타주 재생
        float DeathAnimDuration = PlayAnimMontage(DeathMontage);
        UE_LOG(LogTemp, Warning, TEXT("PlayAnimMontage(DeathMontage) Called! Section: %s"), *SectionName.ToString());
        if (DeathAnimDuration > 0.0f)
        {
            // 애니메이션이 있는 섹션으로 점프
            if (GetMesh()->GetAnimInstance())
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, DeathMontage);
			}
            
            // 5. 애니메이션 길이 + 여유시간(2초) 뒤에 사라지게 설정
            SetLifeSpan(DeathAnimDuration/2 - 1.0f); // 절반 재생 후 사라지게
        }
        else
        {
            // 몽타주가 없거나 실패하면 그냥 3초 뒤 삭제
            SetLifeSpan(0.0f);
        }
    }
    else
    {
        // 몽타주가 없으면 그냥 바로 삭제 예약
        SetLifeSpan(0.0f);
    }
}