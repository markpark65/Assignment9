#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ANGameMode.generated.h"

UCLASS()
class ASSIGNMENT9_API AANGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AANGameMode();
	
protected:
	virtual void BeginPlay() override;

public:
	void ProcessPlayerInput(APlayerController* Player, const FString& InputNumber);
	virtual void PostLogin(APlayerController* NewPlayer) override;
	void PlayerRequestedRestart();
private:

	TArray<int32> TargetNumbers;

	void GenerateRandomNumbers();
	
	bool ValidateInput(const FString& Input, FString& OutErrorMessage);

	FString CheckAnswer(const FString& Input);

	void HandleGameEnd(APlayerController* Winner);

	void ResetGame();

	void CheckDrawCondition();

	// --- 턴 제어 관련 변수 및 함수 ---
	FTimerHandle TurnTimerHandle;
	int32 CurrentPlayerIndex; // 현재 턴인 플레이어의 인덱스
	int32 ReadyPlayersCount = 0;
	void StartNextTurn();
	void OnTimerTicks();
	void HandleTurnTimeout();
};
