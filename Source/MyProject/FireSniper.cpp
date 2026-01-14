// Fill out your copyright notice in the Description page of Project Settings.

#include "FireSniper.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"              // 나이아가라 시스템
#include "NiagaraFunctionLibrary.h"     // 나이아가라 스폰 함수용
#include "NiagaraComponent.h"

// 생성자 이름 변경
AFireSniper::AFireSniper()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFireSniper::BeginPlay()
{
	Super::BeginPlay();
}

void AFireSniper::FireSniper()
{
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }
    FVector StartLocation;
    FRotator CameraRotation;
    GetController()->GetPlayerViewPoint(StartLocation, CameraRotation);

    FVector MuzzleLocation = GetMesh()->GetSocketLocation(TEXT("Muzzle_01"));
    float Range = 10000.0f; 
    FVector EndLocation = StartLocation + (CameraRotation.Vector() * Range);

    // 1. [변경] 여러 개를 담을 배열 선언
    TArray<FHitResult> OutHits;

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic); // 벽
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);        // 적
    ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHitAny = GetWorld()->LineTraceMultiByObjectType(
        OutHits,
        StartLocation,
        EndLocation,
        ObjectParams,
        Params
    );

    // 3. 관통 로직 처리
    // 기본적으로 빔의 끝점은 사거리 끝(허공)으로 설정
    FVector BeamEndPoint = EndLocation; 

    // 중복 피격 방지용 (한 캐릭터에 캡슐, 메쉬 등 여러 개가 맞을 수 있음)
    TSet<AActor*> HitActors; 

    for (const FHitResult& Hit : OutHits)
    {
        AActor* HitActor = Hit.GetActor();
        
        // 이미 처리한 액터면 건너뜀
        if (!HitActor || HitActors.Contains(HitActor)) continue;
        HitActors.Add(HitActor);

        // (A) 벽(WorldStatic)에 맞았을 경우 -> 관통 멈춤!
        // 언리얼 기본 채널: ECC_WorldStatic = 0 (보통)
        // 확실하게 하려면 콜리전 프리셋이나 채널을 확인해야 함. 
        // 여기서는 "Pawn이 아니면 벽"이라고 가정하겠습니다.
        if (!HitActor->IsA(APawn::StaticClass())) 
        {
            BeamEndPoint = Hit.ImpactPoint; // 빔을 벽까지만 그림
            
            // 벽 타격 이펙트
            if (SniperHitEffect)
            {
                UGameplayStatics::SpawnEmitterAtLocation(this, SniperHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), true);
            }
            break; // ★ 루프 탈출 (더 이상 뒤에 있는 건 안 맞음)
        }

        // (B) 적(Pawn)에 맞았을 경우 -> 데미지 주고 통과(Continue)
        UGameplayStatics::ApplyDamage(HitActor, SniperDamage, GetController(), this, UDamageType::StaticClass());

        // 적 타격 이펙트 (피 튀기는 것 등)
        if (SniperHitEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(this, SniperHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), true);
        }
    }

    // 4. 궤적 이펙트 (Trail) 그리기
    if (SniperTrailEffect)
    {
        // 나이아가라 스폰
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SniperTrailEffect,
            MuzzleLocation, 
            FRotator::ZeroRotator,
            FVector(1.f),
            true,
            true,
            ENCPoolMethod::AutoRelease
        );

        if (NiagaraComp)
        {
            // ★ 핵심: 나이아가라에 "BeamEnd"라는 이름으로 변수값을 쏴줍니다.
            // Cascade처럼 모듈 찾고 Type Data 바꾸고 할 필요가 없습니다.
            NiagaraComp->SetVariableVec3(FName("User.Beam End"), BeamEndPoint);
        }
    }
}
