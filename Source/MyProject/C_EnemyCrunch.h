#pragma once

#include "CoreMinimal.h"
#include "C_EnemyBase.h" // AC_EnemyBase를 부모로 지정
#include "C_EnemyCrunch.generated.h"

UCLASS()
class MYPROJECT_API AC_EnemyCrunch : public AC_EnemyBase // AC_EnemyBase 상속
{
	 GENERATED_BODY()

public:
	 AC_EnemyCrunch();
	 virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	 virtual void BeginPlay() override;

public: 
	 virtual void Tick(float DeltaTime) override;
	 
protected:
	// [필수] 부모 클래스 함수 오버라이드
	virtual void Die() override; 
    void Attack();
    // --- 피격/사망 관련 변수 ---
	UPROPERTY(EditAnywhere, Category = "Combat|Montage")
    class UAnimMontage* HitMontage;

    UPROPERTY(EditAnywhere, Category = "Combat|Montage")
    class UAnimMontage* DeathMontage;

	AActor* LastAttacker = nullptr;

	 // --- 콤보/AI 상태 변수 ---
	 UPROPERTY(EditAnywhere, Category = "Combat|Crunch")
	 float AttackRange = 150.0f;

	 UPROPERTY(EditAnywhere, Category = "Combat|Combo")
	 class UAnimMontage* AttackMontage_1; // 1타 몽타주

	 UPROPERTY(EditAnywhere, Category = "Combat|Combo")
	 class UAnimMontage* AttackMontage_2; // 2타 몽타주

	 UPROPERTY(EditAnywhere, Category = "Combat|Combo")
	 class UAnimMontage* AttackMontage_3; // 3타 몽타주

	 UPROPERTY(VisibleAnywhere, Category = "Combat|Combo")
	 int32 ComboStage = 0; // 현재 콤보 단계 (0: 준비, 1~3: 공격 중)

	 // [AI 상태 관리 변수 재선언]
	 bool bIsAttacking = false;
	 FTimerHandle AttackTimerHandle;
	 bool bIsWaiting = false;

	 // --- 내부 함수 ---
	 void AttackCombo(); 
	 void ResetCombo(); 
	 void OnAttackFinished();
	 void OnWaitFinished();

	 // 데미지 판정을 위한 필수 변수
    UPROPERTY(EditAnywhere, Category = "Combat|Melee")
    float MeleeDamage = 30.0f; // 한 타당 피해량

    UPROPERTY(EditAnywhere, Category = "Combat|Melee")
    float MeleeRadius = 150.0f; // 데미지 판정 범위 (Capsule/Sphere)

    // 데미지 판정 중인지 확인하는 플래그
    bool bIsDamageActive = false; 

    void SetDamageActive(bool bActive);

	 // 애니메이션 몽타주에서 호출될 함수 
	 UFUNCTION()
	 void ComboWindowCheck(UAnimMontage* Montage, bool bInterrupted); 
};