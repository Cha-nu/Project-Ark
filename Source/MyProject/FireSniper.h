#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FireSniper.generated.h"

class UNiagaraSystem;
class UParticleSystem;

UCLASS()
class MYPROJECT_API AFireSniper : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자
	AFireSniper();
protected:
    virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireSniper();

protected:
	// class 키워드를 앞에 붙여서 사용 (전방 선언)
	
	// 1. 궤적 (나이아가라 사용)
	UPROPERTY(EditDefaultsOnly, Category = "Sniper FX")
	class UNiagaraSystem* SniperTrailEffect; 

	// 2. 타격 (기존 캐스케이드 사용)
	UPROPERTY(EditDefaultsOnly, Category = "Sniper FX")
	class UParticleSystem* SniperHitEffect; 

	UPROPERTY(EditDefaultsOnly, Category = "Sniper Stats")
	float SniperDamage = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sniper SX")
    class USoundBase* FireSound; // 발사 사운드
};