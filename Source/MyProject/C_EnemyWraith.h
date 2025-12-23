#pragma once

#include "CoreMinimal.h"
#include "C_EnemyBase.h"
#include "C_EnemyWraith.generated.h"

UCLASS()
class MYPROJECT_API AC_EnemyWraith : public AC_EnemyBase
{
	GENERATED_BODY()

public:
	AC_EnemyWraith();
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
	
	// --- Wraith 전용 변수 ---
	UPROPERTY(EditAnywhere, Category = "Combat")
    class UAnimMontage* DeathMontage;
	AActor* LastAttacker = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class AActor> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    FName MuzzleSocketName = "Muzzle_01";

	UPROPERTY(EditAnywhere, Category = "Combat")
    class UAnimMontage* HitMontage;

	bool bIsAttacking = false;
	FTimerHandle AttackTimerHandle;

	void Attack();
	void OnAttackFinished();

	bool bIsWaiting = false;
	void OnWaitFinished();
};