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

	// 각 인스턴스가 가질 고유한 산개 각도
    UPROPERTY(EditAnywhere, Category = "AI")
    float TargetOffsetAngle;

	// 공격 사거리 (베이스에서 관리하면 편리합니다)
    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackRange = 500.0f;

    // [핵심 함수] 플레이어 주변의 분산된 목적지 좌표를 계산
    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetSpreadTargetLocation(AActor* TargetActor);

	// 앞을 가로막는 장애물(아군)을 감지할 거리
    UPROPERTY(EditAnywhere, Category = "AI")
    float DetectionDistance = 250.0f;

    // 가로막혔을 때 회전할 각도 속도 (도/초)
    UPROPERTY(EditAnywhere, Category = "AI")
    float FlankRotationSpeed = 45.0f;

    UPROPERTY(EditAnywhere, Category = "AI")
    bool bIsDead = false;

    // 현재 내가 가로막혀 있는 상태인지 확인
    bool bIsPathBlocked = false;

    // 가로막힘을 감지하고 각도를 업데이트하는 함수
    void UpdateSpreadAngleIfBlocked(float DeltaTime);
};