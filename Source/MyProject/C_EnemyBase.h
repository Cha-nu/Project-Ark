#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_EnemyBase.generated.h"

UCLASS()
class MYPROJECT_API AC_EnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AC_EnemyBase();

protected:
	virtual void BeginPlay() override;

public:	
	// 언리얼 엔진의 표준 데미지 처리 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// 체력 변수 (블루프린트에서도 보고 싶으면 EditAnywhere)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentHealth;

	// 사망 처리 함수
	virtual void Die();
};