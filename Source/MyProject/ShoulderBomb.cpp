// ShoulderBomb.cpp
#include "ShoulderBomb.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h" // ApplyRadialDamage 등을 위해 필수

AShoulderBomb::AShoulderBomb()
{
    PrimaryActorTick.bCanEverTick = false; // 투사체는 틱이 필요 없는 경우가 많아 성능 최적화

    // 1. 충돌체 설정 (Root)
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile")); // 언리얼 기본 프리셋 사용
    CollisionComp->OnComponentHit.AddDynamic(this, &AShoulderBomb::OnHit); // 델리게이트 바인딩
    
    // 플레이어가 밟고 올라가지 못하게 설정
    CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
    CollisionComp->CanCharacterStepUpOn = ECB_No;

    RootComponent = CollisionComp;

    // 2. 메쉬 설정
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 Sphere가 담당하므로 끔

    // 3. 발사체 움직임 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 2000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향으로 회전
    ProjectileMovement->bShouldBounce = false; // 튕기게 할지 (바로 터지게 하려면 false)
}

void AShoulderBomb::BeginPlay()
{
    Super::BeginPlay();

    // [핵심] 생성자(Instigator) 충돌 무시 처리
    // 이 액터를 생성한 폰(Pawn)을 가져옵니다.
    if (GetInstigator()) 
    {
        CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
		GetInstigator()->MoveIgnoreActorAdd(this);
    }
    
    // 안전장치: 3초 뒤에는 무조건 삭제 (메모리 누수 방지)
    SetLifeSpan(3.0f);
}

void AShoulderBomb::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 나 자신이나 주인(Instigator)과 부딪힌 게 아닐 때만 폭발
    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetInstigator()))
    {
        // 1. 폭발 이펙트 재생
        if (ExplosionEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, GetActorLocation());
        }

        // 폭발 사운드 재생
        if (ExplosionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
        }

		TArray<AActor*> IgnoreActors;
        IgnoreActors.Add(this);             // 폭탄 자신
        IgnoreActors.Add(GetInstigator());

        // 2. 범위 대미지 적용 (Radial Damage)
        UGameplayStatics::ApplyRadialDamage(
            this,
            ExplosionDamage,            // Base Damage
            GetActorLocation(),         // Origin
            ExplosionRadius,            // Radius
            nullptr,                    // DamageType (기본)
            IgnoreActors,          // Ignore Actors (비워둠)
            this,                       // DamageCauser
            GetInstigatorController(),  // InstigatorController (중요: 킬 판정용)
            true                        // bDoFullDamage (중심에서 멀어질수록 약해질지 여부: True면 전체 대미지)
        );

        // 3. 투사체 삭제
        Destroy();
    }
}