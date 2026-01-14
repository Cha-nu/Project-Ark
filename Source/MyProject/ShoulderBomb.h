// ShoulderBomb.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShoulderBomb.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;

UCLASS()
class MYPROJECT_API AShoulderBomb : public AActor
{
    GENERATED_BODY()
    
public:    
    AShoulderBomb();

protected:
    virtual void BeginPlay() override;

    // --- Components ---
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* CollisionComp; // 충돌 감지용 (루트)

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComp; // 눈에 보이는 메쉬

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement; // 날아가는 물리 로직

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ExplosionRadius = 300.0f; // 폭발 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ExplosionDamage = 80.0f; // 대미지

    UPROPERTY(EditAnywhere, Category = "Combat")
    UParticleSystem* ExplosionEffect; // 폭발 이펙트 (Cascade) 
    
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    class USoundBase* ExplosionSound; // 폭발 사운드

    // --- Functions ---
    // 충돌 시 호출될 함수 (UFUNCTION 필수)
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};