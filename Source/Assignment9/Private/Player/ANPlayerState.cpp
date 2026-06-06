#include "Player/ANPlayerState.h"
#include "Net/UnrealNetwork.h"

AANPlayerState::AANPlayerState()
{
	CurrentAttempts = 0;
	MaxAttempts = 3;
	bReplicates = true;
}

void AANPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AANPlayerState, CurrentAttempts);
}

void AANPlayerState::AddAttempt()
{
	if (HasAuthority())
	{
		CurrentAttempts++;
	}
}

void AANPlayerState::ResetAttempts()
{
	if (HasAuthority())
	{
		CurrentAttempts = 0;
	}
}

void AANPlayerState::OnRep_CurrentAttempts()
{
	UE_LOG(LogTemp, Log, TEXT("UI 업데이트 - 현재 시도 횟수: %d / %d"), CurrentAttempts, MaxAttempts);

}


