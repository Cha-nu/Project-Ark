#include "C_MyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"

AC_MyGameMode::AC_MyGameMode()
{
    // 필요하다면 기본 설정
}

void AC_MyGameMode::BeginPlay()
{
    Super::BeginPlay();

    CurrentSpawnCount = 0;
    DeadEnemyCount = 0;

    // 1. 스폰 타이머 시작 (10초마다 반복)
    GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AC_MyGameMode::SpawnEnemy, SpawnInterval, true);
}

void AC_MyGameMode::SpawnEnemy()
{
    // 2. 최대 스폰 수 도달 시 타이머 정지
    if (CurrentSpawnCount >= MaxSpawnCount)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    if (EnemyClasses.Num() > 0)
    {
        // 3. 적 랜덤 선택 (또는 순서대로)
        int32 Index = CurrentSpawnCount % EnemyClasses.Num(); // 0, 1, 2 순서대로 반복
        TSubclassOf<ACharacter> EnemyClass = EnemyClasses[Index];

        if (EnemyClass)
        {
            FVector SpawnLocation = FVector::ZeroVector;
            FRotator SpawnRotation = FRotator::ZeroRotator;

            // A. "EnemySpawn" 태그를 가진 모든 액터를 찾습니다.
            TArray<AActor*> SpawnPoints;
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EnemySpawn"), SpawnPoints);

            // B. 스폰 포인트가 하나라도 있다면 그 중 하나를 랜덤으로 뽑습니다.
            if (SpawnPoints.Num() > 0)
            {
                int32 PointIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
                AActor* ChosenPoint = SpawnPoints[PointIndex];
                
                SpawnLocation = ChosenPoint->GetActorLocation();
                SpawnRotation = ChosenPoint->GetActorRotation(); // 방향도 설정해둔 대로 나옵니다.
            }
            else
            {
                // C. 만약 스폰 포인트를 못 찾았다면? (예외 처리: 플레이어 근처 랜덤)
                UE_LOG(LogTemp, Error, TEXT("No Actors with tag 'EnemySpawn' found! Spawning randomly."));
                SpawnLocation = FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 200);
            }

            // ---------------------------------------------------------

            // 적 생성
            GetWorld()->SpawnActor<ACharacter>(EnemyClass, SpawnLocation, SpawnRotation);
            
            CurrentSpawnCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned Enemy %d / %d at %s"), CurrentSpawnCount, MaxSpawnCount, *SpawnLocation.ToString());
        }
    }
}

void AC_MyGameMode::OnEnemyKilled()
{
    DeadEnemyCount++;
    UE_LOG(LogTemp, Warning, TEXT("Enemy Killed! Dead: %d / Total: %d"), DeadEnemyCount, MaxSpawnCount);

    // 4. 승리 조건: 스폰도 다 끝났고(18마리), 죽은 적도 18마리라면
    if (CurrentSpawnCount >= MaxSpawnCount && DeadEnemyCount >= MaxSpawnCount)
    {
        GoToGameClear();
    }
}

void AC_MyGameMode::OnPlayerDied()
{
    GoToGameOver();
}

void AC_MyGameMode::GoToGameClear()
{
    UE_LOG(LogTemp, Warning, TEXT("GAME CLEAR!"));
    UGameplayStatics::OpenLevel(this, FName("L_Clear"));
}

void AC_MyGameMode::GoToGameOver()
{
    UE_LOG(LogTemp, Error, TEXT("GAME OVER!"));
    UGameplayStatics::OpenLevel(this, FName("L_GameOver"));
}