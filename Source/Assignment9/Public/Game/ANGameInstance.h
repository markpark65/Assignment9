#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ANGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHostCreatedLobbyDelegate);
UCLASS()
class ASSIGNMENT9_API UANGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UANGameInstance();
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, Category = "GameSettings")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "GameSettings")
	bool bIsMultiplayer;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void AutoMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySessionAndLeave();

	// 방장이 세션 생성을 완료했을 때 블루프린트로 신호를 보내는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnHostCreatedLobbyDelegate OnHostLobbyCreated;

private:
	// 세션 관리 포인터
	IOnlineSessionPtr OnlineSessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	// 엔진이 통신을 마치고 부를 콜백 함수들
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};
