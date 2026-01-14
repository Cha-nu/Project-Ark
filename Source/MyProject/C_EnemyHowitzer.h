#pragma once

#include "CoreMinimal.h"
#include "C_EnemyBase.h" // 부모 클래스: AC_EnemyBase 상속
#include "C_EnemyHowitzer.generated.h"

UCLASS()
class MYPROJECT_API AC_EnemyHowitzer : public AC_EnemyBase // AC_EnemyBase 상속
{
	 GENERATED_BODY()

public:
	 AC_EnemyHowitzer();
	 virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	 virtual void BeginPlay() override;

public: 
	 virtual void Tick(float DeltaTime) override;

protected:
    // --- Howitzer 전용 변수 ---

    UPROPERTY(EditAnywhere, Category = "Combat|Howitzer")
    class UAnimMontage* AttackMontage;

    // 총구 섬광
    UPROPERTY(EditAnywhere, Category = "Effects")
    class UParticleSystem* MuzzleFlashParticle;
    
    // [Wraith에서 이식]: 발사체, 소켓, 피격/사망 몽타주
    UPROPERTY(EditAnywhere, Category = "Combat|Howitzer")
    TSubclassOf<class AActor> ProjectileClass; // 유탄 Bolt Actor 클래스 (BP)

    UPROPERTY(EditAnywhere, Category = "Combat|Howitzer")
    FName MuzzleSocketName = "Muzzle_01"; // Howitzer의 총구 소켓 이름 (확인 필요)

    UPROPERTY(EditAnywhere, Category = "Combat|Howitzer")
    class UAnimMontage* HitMontage; // 피격 모션

    UPROPERTY(EditAnywhere, Category = "Combat|Howitzer")
    class UAnimMontage* DeathMontage; // 사망 모션
    
    // 피격 방향 계산을 위한 마지막 공격자 저장
    AActor* LastAttacker = nullptr;

    bool bIsAttacking = false;
    FTimerHandle AttackTimerHandle;

    // --- 내부 함수 ---
    // 기존 Attack 함수는 발사 로직으로 대체됩니다.
    void Attack();
    void OnAttackFinished();

    // [이식] 부모 클래스의 사망 로직 재정의
    virtual void Die() override; 
    
    bool bIsWaiting = false;
    void OnWaitFinished();

    // 경직 시간을 관리할 핸들
    FTimerHandle HitStunTimerHandle;

    // 경직 지속 시간 (초 단위, 필요에 따라 조정하세요)
    UPROPERTY(EditAnywhere, Category = "Combat")
    float HitStunDuration = 0.5f;

    // 이동을 다시 허용하는 함수
    void ResetMovement();
};