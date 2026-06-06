#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ANPlayerController.generated.h"

UCLASS()
class ASSIGNMENT9_API AANPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Baseball")
	void Server_SubmitGuess(const FString& Guess);


	UFUNCTION(Client, Reliable)
	void Client_ShowResult(const FString& ResultMsg);


	UFUNCTION(Client, Reliable)
	void Client_ShowError(const FString& ErrorMsg);


	UFUNCTION(Client, Reliable)
	void Client_GameEnd(const FString& EndMsg);

	UFUNCTION(Client, Reliable)
	void Client_ClearUI();
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Baseball|UI")
	void UpdateUI_Result(const FString& ResultMsg);

	UFUNCTION(BlueprintImplementableEvent, Category = "Baseball|UI")
	void UpdateUI_Error(const FString& ErrorMsg);

	UFUNCTION(BlueprintImplementableEvent, Category = "Baseball|UI")
	void UpdateUI_GameEnd(const FString& EndMsg);

	UFUNCTION(BlueprintImplementableEvent, Category = "Baseball|UI")
	void UpdateUI_Clear();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Baseball")
	void Server_RequestRestart();
};
