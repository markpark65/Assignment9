#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ANPlayerState.generated.h"

UCLASS()
class ASSIGNMENT9_API AANPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AANPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// UI(위젯)에서 현재 횟수와 최대 횟수를 가져다 쓰기 위한 Getter 함수
	UFUNCTION(BlueprintCallable, Category = "Baseball")
	int32 GetCurrentAttempts() const { return CurrentAttempts; }

	UFUNCTION(BlueprintCallable, Category = "Baseball")
	int32 GetMaxAttempts() const { return MaxAttempts; }

	// 서버에서 호출하여 시도 횟수를 증가시키거나 초기화
	void AddAttempt();
	void ResetAttempts();

protected:
	// 현재 시도 횟수 (서버에서 클라이언트로 복제됨)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAttempts)
	int32 CurrentAttempts;

	// 최대 시도 횟수 (과제 기준 3회)
	int32 MaxAttempts;

	// CurrentAttempts 값이 클라이언트 측에 복제되었을 때 자동으로 실행되는 함수
	UFUNCTION()
	void OnRep_CurrentAttempts();
};
