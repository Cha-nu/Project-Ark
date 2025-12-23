#pragma once

#include "CoreMinimal.h"
#include "C_EnemyBase.h" // 부모 클래스: AC_EnemyBase 상속
#include "C_EnemyGRIMexe.generated.h"

UCLASS()
class MYPROJECT_API AC_EnemyGRIMexe : public AC_EnemyBase // AC_EnemyBase 상속
{
	GENERATED_BODY()

public:
	AC_EnemyGRIMexe();

protected:
	virtual void BeginPlay() override;

public: 
	virtual void Tick(float DeltaTime) override;

protected:
	// --- GRIMexe 전용 변수 ---
	UPROPERTY(EditAnywhere, Category = "Combat|GRIMexe")
	float AttackRange = 650.0f; // Wraith(500f)와 Howitzer(800f)의 중간 사거리

	UPROPERTY(EditAnywhere, Category = "Combat|GRIMexe")
	class UAnimMontage* AttackMontage;

	bool bIsAttacking = false;
	FTimerHandle AttackTimerHandle;

	// --- 내부 함수 ---
	void Attack();
	void OnAttackFinished();

	bool bIsWaiting = false;
	void OnWaitFinished();
};