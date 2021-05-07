// Fill out your copyright notice in the Description page of Project Settings.


#include "RawDecoder.h"

// Sets default values
ARawDecoder::ARawDecoder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARawDecoder::PushData(TArray<uint8> Data, long TimeStamp)
{
	if (!Decoder)
	{
		DecoderParams.VerboseDebug = true;
		DecoderParams.AllowBuffering = false;
		DecoderParams.DoubleDecodeKeyframe = false;
		DecoderParams.DrainOnKeyframe = false;
		DecoderParams.LowPowerMode = false;
		DecoderParams.DropBadFrames = false;
		DecoderParams.DecodeSei = false;
		UE_LOG(LogTemp, Warning, TEXT("Creating new decoder..."));
		Decoder = new UUEPopH264::Decoder(DecoderParams, ThreadedDecoding);
	}

	UE_LOG(LogTemp, Warning, TEXT("pushing x %i"), Data.Num());
	Decoder->PushFrameData(Data, (int)TimeStamp);
}

// Called when the game starts or when spawned
void ARawDecoder::BeginPlay()
{
	Super::BeginPlay();
	FetchRawH264HTTP("https://github.com/NewChromantics/PopH264/raw/master/Unity/PopH264/Assets/cat.h264.bytes");

}

void ARawDecoder::OnRawH264HTTPReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool wasSuccessful)
{
	if (wasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Web request to raw video was successful"));
		PushData(Response->GetContent(), 0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Web request to raw video was unsuccessful"));
	}
}

// Called every frame
void ARawDecoder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Decoder == nullptr)
		return;

	/*int NewFrame = Decoder->GetNextFrame(FramePlanes, FramePlaneFormats);
	if (NewFrame != 0) {
		UE_LOG(LogTemp, Warning, TEXT("Decoded Frame %i"), NewFrame);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Either frame is zero or frame was not decoded"));
	}*/
}

void ARawDecoder::FetchRawH264HTTP(FString URL)
{
	UE_LOG(LogTemp, Warning, TEXT("Fetching h264 data"));
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &ARawDecoder::OnRawH264HTTPReceived);
	request->SetURL(URL);
	request->SetVerb("GET");
	request->ProcessRequest();
}

