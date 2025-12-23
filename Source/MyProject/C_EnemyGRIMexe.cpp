#include "C_EnemyGRIMexe.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "GameFramework/CharacterMovementComponent.h"

AC_EnemyGRIMexe::AC_EnemyGRIMexe()
{
	 PrimaryActorTick.bCanEverTick = true;

	 // [이동 설정] (Wraith보다 느리고 Howitzer보다 빠르도록 설정)
	 bUseControllerRotationYaw = false; 
	 GetCharacterMovement()->bOrientRotationToMovement = true; 
	 GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
	 GetCharacterMovement()->MaxWalkSpeed = 500.0f; // Wraith 600f, Crunch 450f, Howitzer 350f
}

void AC_EnemyGRIMexe::BeginPlay()
{
	 Super::BeginPlay(); // AC_EnemyBase의 BeginPlay(체력 초기화) 실행됨

	 // [대기 로직 유지]
	 bIsWaiting = true;
	 GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyGRIMexe::OnWaitFinished, 5.0f, false);
}

void AC_EnemyGRIMexe::OnWaitFinished()
{
	 bIsWaiting = false;
}

void AC_EnemyGRIMexe::Tick(float DeltaTime)
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

void AC_EnemyGRIMexe::Attack()
{
	 if (bIsAttacking || !AttackMontage) return;

	 bIsAttacking = true;

	 AAIController* AIC = Cast<AAIController>(GetController());
	 if (AIC) AIC->StopMovement();

	 float Duration = PlayAnimMontage(AttackMontage);
	 float WaitTime = (Duration > 0.0f) ? Duration : 1.0f;

	 GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AC_EnemyGRIMexe::OnAttackFinished, WaitTime, false);
}

void AC_EnemyGRIMexe::OnAttackFinished()
{
	 bIsAttacking = false;
}