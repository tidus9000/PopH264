// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UEPopH264.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "GameFramework/Actor.h"
#include "RawDecoder.generated.h"

UCLASS()
class POPH264_API ARawDecoder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	void PushData(TArray<uint8> Data, long TimeStamp);

	ARawDecoder();
	bool ThreadedDecoding;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UUEPopH264::Decoder* Decoder = nullptr;
	TArray<UTexture2D*> FramePlanes;
	TArray<PixelFormat> FramePlaneFormats;
	FDecoderParams DecoderParams;
	void OnRawH264HTTPReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool wasSuccessful);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void FetchRawH264HTTP(FString URL);

};
