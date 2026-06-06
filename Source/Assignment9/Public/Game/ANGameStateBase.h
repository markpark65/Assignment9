#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ANGameStateBase.generated.h"

class AANPlayerState;
UCLASS()
class ASSIGNMENT9_API AANGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AANGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 모든 클라이언트의 UI에 표시될 남은 시간 (턴당 30초)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TurnSystem")
	int32 RemainingTime;

	// 현재 입력 권한을 가진 플레이어
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TurnSystem")
	AANPlayerState* CurrentTurnPlayer;
};
