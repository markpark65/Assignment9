#include "Game/ANGameMode.h"
#include "Game/ANGameStateBase.h"
#include "Game/ANGameInstance.h"
#include "Player/ANPlayerController.h"
#include "Player/ANPlayerState.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

AANGameMode::AANGameMode()
{
	GameStateClass = AANGameStateBase::StaticClass();
	PlayerStateClass = AANPlayerState::StaticClass();
	PlayerControllerClass = AANPlayerController::StaticClass();
	CurrentPlayerIndex = -1;
}

void AANGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void AANGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AANGameStateBase* GS = GetGameState<AANGameStateBase>();

	bool bIsMulti = (GetWorld()->GetNetMode() != NM_Standalone);

	if (bIsMulti && GS)
	{
		int32 PlayerCount = GS->PlayerArray.Num();

		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("멀티플레이 현재 방 인원: %d명"), PlayerCount));

		if (PlayerCount == 2)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("2명 접속 완료! 게임을 시작합니다."));
			FTimerHandle StartTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &AANGameMode::ResetGame, 1.0f, false);
		}
	}
	else if (!bIsMulti)
	{
		FTimerHandle StartTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &AANGameMode::ResetGame, 0.5f, false);
	}
}

void AANGameMode::GenerateRandomNumbers()
{
	TargetNumbers.Empty();

	while (TargetNumbers.Num() < 3)
	{
		int32 RandomNum = FMath::RandRange(1, 9);
		if (!TargetNumbers.Contains(RandomNum))
		{
			TargetNumbers.Add(RandomNum);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("서버에 생성된 정답: %d%d%d"), TargetNumbers[0], TargetNumbers[1], TargetNumbers[2]);
}

bool AANGameMode::ValidateInput(const FString& Input, FString& OutErrorMessage)
{
	// 1. 3자리인지 확인
	if (Input.Len() != 3)
	{
		OutErrorMessage = TEXT("3자리 숫자를 입력해주세요.");
		return false;
	}

	TArray<TCHAR> CheckedChars;

	for (int32 i = 0; i < Input.Len(); ++i)
	{
		TCHAR CurrentChar = Input[i];

		// 2. 문자가 포함되어 있는지 (숫자인지) 확인, 0은 제외 (1~9)
		if (!FChar::IsDigit(CurrentChar) || CurrentChar == '0')
		{
			OutErrorMessage = TEXT("1에서 9 사이의 숫자만 입력 가능합니다.");
			return false;
		}

		// 3. 중복되는 숫자가 있는지 확인
		if (CheckedChars.Contains(CurrentChar))
		{
			OutErrorMessage = TEXT("중복되지 않은 숫자를 입력해주세요.");
			return false;
		}

		CheckedChars.Add(CurrentChar);
	}

	return true;
}

FString AANGameMode::CheckAnswer(const FString& Input)
{
	int32 Strikes = 0;
	int32 Balls = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		// 입력받은 TCHAR를 int32로 변환
		int32 InputDigit = Input[i] - '0';

		if (TargetNumbers[i] == InputDigit)
		{
			Strikes++; // 자리와 숫자가 모두 일치
		}
		else if (TargetNumbers.Contains(InputDigit))
		{
			Balls++; // 숫자는 있지만 자리가 다름
		}
	}

	if (Strikes == 0 && Balls == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS %dB"), Strikes, Balls);
}

void AANGameMode::ProcessPlayerInput(APlayerController* Player, const FString& InputNumber)
{
	AANGameStateBase* GS = GetGameState<AANGameStateBase>();
	AANPlayerController* PC = Cast<AANPlayerController>(Player);
	AANPlayerState* PS = Player ? Player->GetPlayerState<AANPlayerState>() : nullptr;

	if (!PC || !PS || !GS) return;

	FString CleanInput = InputNumber.Replace(TEXT(" "), TEXT(""));

	if (GS->CurrentTurnPlayer != PS)
	{
		PC->Client_ShowError(TEXT("아직 당신의 턴이 아닙니다. 기다려주세요!"));
		return; // 로직 종료 (서버가 받아주지 않음)
	}

	if (PS->GetCurrentAttempts() >= PS->GetMaxAttempts())
	{
		PC->Client_ShowError(TEXT("남은 기회가 없습니다. 다른 플레이어를 기다려주세요."));
		return;
	}
	FString ErrorMessage;

	if (!ValidateInput(CleanInput, ErrorMessage))
	{
		PC->Client_ShowError(ErrorMessage);
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	PS->AddAttempt();

	FString Result = CheckAnswer(CleanInput);
	PC->Client_ShowResult(Result);

	if (Result == TEXT("3S 0B"))
	{
		PC->Client_ShowResult(Result);
		HandleGameEnd(Player);
	}
	else
	{
		CheckDrawCondition();
		StartNextTurn();
	}
}

void AANGameMode::CheckDrawCondition()
{
	int32 ValidPlayers = 0;
	int32 FinishedPlayers = 0;

	// 서버에 접속한 모든 플레이어 순회
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* Player = It->Get())
		{
			if (AANPlayerState* PS = Player->GetPlayerState<AANPlayerState>())
			{
				ValidPlayers++;
				if (PS->GetCurrentAttempts() >= PS->GetMaxAttempts())
				{
					FinishedPlayers++; // 기회를 다 쓴 플레이어 1명 추가
				}
			}
		}
	}
	if (ValidPlayers > 0 && ValidPlayers == FinishedPlayers)
	{
		HandleGameEnd(nullptr);
	}
}

void AANGameMode::StartNextTurn()
{
	AANGameStateBase* GS = GetGameState<AANGameStateBase>();
	if (!GS || GS->PlayerArray.Num() == 0) return;

	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	CurrentPlayerIndex++;
	if (CurrentPlayerIndex >= GS->PlayerArray.Num())
	{
		CurrentPlayerIndex = 0;
	}

	GS->CurrentTurnPlayer = Cast<AANPlayerState>(GS->PlayerArray[CurrentPlayerIndex]);

	if (GS->CurrentTurnPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("심각한 에러: PlayerState 캐스팅 실패! 현재 스폰된 클래스 이름: %s"), *GS->PlayerArray[CurrentPlayerIndex]->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("정상 캐스팅 됨. 새로운 턴: %s"), *GS->CurrentTurnPlayer->GetPlayerName());
	}

	GS->RemainingTime = 30;
	GetWorld()->GetTimerManager().SetTimer(TurnTimerHandle, this, &AANGameMode::OnTimerTicks, 1.0f, true);
}

void AANGameMode::OnTimerTicks()
{
	AANGameStateBase* GS = GetGameState<AANGameStateBase>();
	if (!GS) return;

	GS->RemainingTime--; // 1초 감소

	if (GS->RemainingTime <= 0)
	{
		HandleTurnTimeout(); // 0초가 되면 타임아웃 처리
	}
}

void AANGameMode::HandleTurnTimeout()
{
	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	AANGameStateBase* GS = GetGameState<AANGameStateBase>();
	if (GS && GS->CurrentTurnPlayer)
	{
		// 시간 내에 입력하지 않았으므로 기회 1회 차감 (활동 체크 및 패널티)
		GS->CurrentTurnPlayer->AddAttempt();

		// 클라이언트들에게 시간 초과 알림 (UI 연동용)
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AANPlayerController* PC = Cast<AANPlayerController>(It->Get()))
			{
				PC->Client_ShowError(FString::Printf(TEXT("%s님의 시간 초과! 기회가 1회 소진됩니다."), *GS->CurrentTurnPlayer->GetPlayerName()));
			}
		}
	}

	// 조기 무승부 체크 후, 게임이 안 끝났으면 턴을 넘김
	CheckDrawCondition();
	StartNextTurn();
}


void AANGameMode::HandleGameEnd(APlayerController* Winner)
{
	FString EndMsg;
	bool bIsMulti = (GetWorld()->GetNetMode() != NM_Standalone);
	if (Winner)
	{
		AANPlayerState* WinnerPS = Winner->GetPlayerState<AANPlayerState>();
		FString WinnerName = WinnerPS ? WinnerPS->GetPlayerName() : TEXT("알 수 없는 플레이어");
		EndMsg = FString::Printf(TEXT("%s 승리!"), *WinnerName);
	}
	else
	{
		if (bIsMulti) {
			EndMsg = TEXT("무승부! 모든 플레이어가 기회를 소진했습니다.");
		}
		else {
			EndMsg = TEXT("실패! 기회를 모두 소진했습니다.");
		}
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AANPlayerController* PC = Cast<AANPlayerController>(It->Get()))
		{
			PC->Client_GameEnd(EndMsg);
		}
	}

	// 공지 후 게임 리셋
	ResetGame();
}

void AANGameMode::ResetGame()
{
	// 정답 초기화 및 재생성
	GenerateRandomNumbers();
	CurrentPlayerIndex = -1;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AANPlayerController* PC = Cast<AANPlayerController>(It->Get()))
		{
			PC->Client_ClearUI();

			if (AANPlayerState* PS = PC->GetPlayerState<AANPlayerState>())
			{
				PS->ResetAttempts();
			}
		}
	}
	StartNextTurn();
}

void AANGameMode::PlayerRequestedRestart()
{
	//UANGameInstance* GI = Cast<UANGameInstance>(GetGameInstance());
	//bool bIsMulti = GI ? GI->bIsMultiplayer : false;
	bool bIsMulti = (GetWorld()->GetNetMode() != NM_Standalone);
	// 싱글이면 1명, 멀티면 2명이 눌러야 시작
	int32 RequiredPlayers = bIsMulti ? 2 : 1;

	ReadyPlayersCount++;
	UE_LOG(LogTemp, Warning, TEXT("재경기 준비 완료: %d / %d"), ReadyPlayersCount, RequiredPlayers);

	if (ReadyPlayersCount >= RequiredPlayers)
	{
		ReadyPlayersCount = 0; // 다음 판을 위해 초기화

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AANPlayerController* PC = Cast<AANPlayerController>(It->Get()))
			{
				PC->Client_ClearUI();
			}
		}

		// 새 게임 시작
		ResetGame();
	}
}