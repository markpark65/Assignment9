#include "Game/ANGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

UANGameInstance::UANGameInstance()
{
}

void UANGameInstance::Init()
{
	Super::Init();

	// 엔진의 온라인 서브시스템 가져와서 연결하기
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		OnlineSessionInterface = Subsystem->GetSessionInterface();
		if (OnlineSessionInterface.IsValid())
		{
			OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
			OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
			OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
		}
	}
}

void UANGameInstance::AutoMatchmaking()
{
	if (OnlineSessionInterface.IsValid())
	{
		EOnlineSessionState::Type SessionState = OnlineSessionInterface->GetSessionState(NAME_GameSession);
		if (SessionState != EOnlineSessionState::NoSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("기존 세션이 감지되었습니다. 세션을 파괴합니다."));
			OnlineSessionInterface->DestroySession(NAME_GameSession);
			return;
		}

		// 방 찾기 세팅
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = true; // 로컬(LAN) 테스트용
		SessionSearch->MaxSearchResults = 10;
		SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);

		UE_LOG(LogTemp, Warning, TEXT("세션 검색을 시작합니다..."));
		OnlineSessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UANGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	//검색 완료 결과 처리
	if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("방을 찾았습니다! 손님으로 자동 접속합니다."));

		// 첫 번째로 찾은 방에 참가
		OnlineSessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("방이 없습니다. 방장이 되어 새로운 방을 생성합니다."));

		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;
		SessionSettings.NumPublicConnections = 2; // 방장 1명 + 손님 1명 (최대 2인)
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bAllowJoinInProgress = true; // 게임 중 난입 허용

		OnlineSessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}
}

void UANGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// 방 생성이 완료되면 블루프린트로 로비를 띄우라고 신호(Broadcast)를 보냄
		UE_LOG(LogTemp, Warning, TEXT("방 생성 완료! 방장 화면에 로비를 띄웁니다."));
		UGameplayStatics::OpenLevel(this, FName("InGameMap"), true, TEXT("listen"));
	}
}

void UANGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		//방 참가 완료 시, 로비 없이 즉시 방장이 있는 방으로 텔레포트
		UE_LOG(LogTemp, Warning, TEXT("방 참가 완료! 방장에게 이동합니다."));
		FString Address;
		if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
		{
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC) PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UANGameInstance::DestroySessionAndLeave()
{
	// 방을 깔끔하게 폭파하고 메인 메뉴로 복귀
	SessionSearch = nullptr;
	if (OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}
	UGameplayStatics::OpenLevel(this, FName("MainMapMap"));
}
void UANGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("세션 파괴 성공! 메인 메뉴로 돌아갈 준비가 되었습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("세션 파괴 실패."));
	}
}