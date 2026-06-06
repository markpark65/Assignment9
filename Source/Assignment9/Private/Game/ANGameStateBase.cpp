#include "Game/ANGameStateBase.h"
#include "Net/UnrealNetwork.h"

AANGameStateBase::AANGameStateBase()
{
	RemainingTime = 30;
	CurrentTurnPlayer = nullptr;
}

void AANGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 두 변수가 서버에서 클라이언트로 자동 동기화되도록 등록
	DOREPLIFETIME(AANGameStateBase, RemainingTime);
	DOREPLIFETIME(AANGameStateBase, CurrentTurnPlayer);
}
