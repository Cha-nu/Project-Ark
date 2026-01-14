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

	// 모든 적이 생성될 때 각자 다른 각도를 부여받아 일직선을 방지
    TargetOffsetAngle = FMath::RandRange(0.0f, 30.0f);
}

float AC_EnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if(bIsDead)
    {
        return 0.0f; // 이미 죽은 적은 데미지 무시
    }

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

FVector AC_EnemyBase::GetSpreadTargetLocation(AActor* TargetActor)
{
    if (!TargetActor) return GetActorLocation();

    // 1. 플레이어 위치 기준, 사거리의 80% 정도 지점을 기본 오프셋으로 설정
    FVector Offset = FVector(AttackRange * 0.8f, 0.0f, 0.0f);

    // 2. 내 고유 각도(TargetOffsetAngle)만큼 Z축 기준으로 회전 (Vector Math)
    // 수식: V' = V.RotateAngleAxis(Angle, Axis)
    Offset = Offset.RotateAngleAxis(TargetOffsetAngle, FVector(0.0f, 0.0f, 1.0f));

    // 3. 플레이어 위치에 오프셋을 더해 최종 목적지 반환
    return TargetActor->GetActorLocation() + Offset;
}

void AC_EnemyBase::UpdateSpreadAngleIfBlocked(float DeltaTime)
{
    // 1. LineTrace 설정
    FHitResult Hit;
    FVector Start = GetActorLocation();
    // 캐릭터가 바라보는 방향(Forward)으로 탐지 거리만큼 발사
    FVector End = Start + (GetActorForwardVector() * DetectionDistance);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // 자기 자신은 무시

    // 2. 전방 탐색 (Pawn 채널 사용)
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);

    if (bHit && Hit.GetActor())
    {
        // 3. 만약 앞에 있는 것이 같은 Enemy 클래스라면?
        if (Hit.GetActor()->IsA(AC_EnemyBase::StaticClass()))
        {
            // 가로막힘 감지! 각도를 변경하여 목적지 슬롯을 옆으로 이동시킴
            // 시계 방향으로 회전시키거나, 상황에 따라 좌우를 결정할 수 있음
            TargetOffsetAngle += FlankRotationSpeed * DeltaTime;

            // 360도 보정 (Clamping)
            if (TargetOffsetAngle >= 360.0f) TargetOffsetAngle -= 360.0f;
            
            bIsPathBlocked = true;
            return;
        }
    }

    bIsPathBlocked = false;
}