#include "C_EnemyBase.h"
#include "Engine/Engine.h"
#include "C_MyGameMode.h"
#include "AIController.h"

AC_EnemyBase::AC_EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true; // Tick이 켜져 있어야 추적함!

    // [핵심] 스폰된 적에게도 AI 컨트롤러를 빙의시킴
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    
    // 기본 AI 컨트롤러 클래스 지정 (혹시 모르니 설정)
    AIControllerClass = AAIController::StaticClass();
}

void AC_EnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

float AC_EnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 체력 깎기
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);


	// 사망 확인
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AC_EnemyBase::Die()
{
	AC_MyGameMode* GM = Cast<AC_MyGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->OnEnemyKilled();
    }
	// 사망 로직
	SetActorEnableCollision(false);
	Destroy();
}