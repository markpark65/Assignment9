#include "Player/ANPlayerController.h"
#include "Game/ANGameMode.h"
#include "Kismet/GameplayStatics.h"

bool AANPlayerController::Server_SubmitGuess_Validate(const FString& Guess)
{
	return true;
}

void AANPlayerController::Server_SubmitGuess_Implementation(const FString& Guess)
{
	AANGameMode* GM = Cast<AANGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->ProcessPlayerInput(this, Guess);
	}
}
void AANPlayerController::Client_ShowResult_Implementation(const FString& ResultMsg)
{
	UpdateUI_Result(ResultMsg);
}

void AANPlayerController::Client_ShowError_Implementation(const FString& ErrorMsg)
{
	UpdateUI_Error(ErrorMsg);
}

void AANPlayerController::Client_GameEnd_Implementation(const FString& EndMsg)
{
	UpdateUI_GameEnd(EndMsg);
}
void AANPlayerController::Server_RequestRestart_Implementation()
{
	AANGameMode* GM = Cast<AANGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->PlayerRequestedRestart();
	}
}

void AANPlayerController::Client_ClearUI_Implementation()
{
	UpdateUI_Clear();

	/*SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);*/
}