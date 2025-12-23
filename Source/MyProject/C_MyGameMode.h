#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "C_MyGameMode.generated.h"

UCLASS()
class MYPROJECT_API AC_MyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
    AC_MyGameMode();

protected:
    virtual void BeginPlay() override;

public:
    // [기능 1] 적이 죽었을 때 호출될 함수
    void OnEnemyKilled();

    // [기능 2] 플레이어 사망 시 호출될 함수
    void OnPlayerDied();

protected:
    // --- 스폰 설정 ---
    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    TArray<TSubclassOf<class ACharacter>> EnemyClasses; // 스폰할 적 목록 (Wraith, Crunch, Howitzer)

    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    float SpawnInterval = 10.0f; // 스폰 간격

    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    int32 MaxSpawnCount = 10; // 3분 / 10초 = 18마리

    // --- 내부 상태 ---
    FTimerHandle SpawnTimerHandle;
    int32 CurrentSpawnCount = 0;
    int32 DeadEnemyCount = 0;

    // --- 스폰 함수 ---
    void SpawnEnemy();
    
    // --- 레벨 이동 ---
    void GoToGameClear();
    void GoToGameOver();
};