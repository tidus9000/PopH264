// Fill out your copyright notice in the Description page of Project Settings.


#include "UEPopH264.h"
#include <iostream>


// Sets default values for this component's properties
UUEPopH264::UUEPopH264()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUEPopH264::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UUEPopH264::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UUEPopH264::Decoder::Decoder(FDecoderParams _DecoderParams, bool _ThreadedDecoding)
{
	int32_t Version = popH264DLL.PopH264_GetVersion();
	int32_t Version2 = PopH264_GetVersion();
	UE_LOG(LogTemp, Warning, TEXT("PopH264 version %i"), Version);
	UE_LOG(LogTemp, Warning, TEXT("PopH264 version %i"), Version2);
	this->ThreadedDecoding = _ThreadedDecoding;

	if (&_DecoderParams == nullptr)
		_DecoderParams = FDecoderParams();

	TArray<uint8> ErrorBuffer;
	//TSharedRef<FJsonObject> OutJson = MakeShared<FJsonObject>();
	FString OutJson;
	FJsonObjectConverter::UStructToJsonObjectString(_DecoderParams, OutJson, 0, 0);
	OutJson = "{ \"Decoder\":\"\",\"VerboseDebug\" : false,\"AllowBuffering\" : false,\"DoubleDecodeKeyframe\" : false,\"DrainOnKeyframe\" : false,\"LowPowerMode\" : false,\"DropBadFrames\" : false,\"DecodeSei\" : false }";
	//Instance = PopH264_CreateDecoder(TCHAR_TO_ANSI(*OutJson), (char*)ErrorBuffer.GetData(), ErrorBuffer.Num());
	Instance = popH264DLL.PopH264_CreateDecoder(TCHAR_TO_ANSI(*OutJson), (char*)ErrorBuffer.GetData(), ErrorBuffer.Num());
	UE_LOG(LogTemp, Warning, TEXT("Instance create"));
}

bool UUEPopH264::Decoder::PushFrameData(TArray<uint8> H264Data, int FrameNumber)
{
	FFrameInput NewFrame;
	NewFrame.FrameNumber = FrameNumber;
	NewFrame.Bytes = H264Data;
	return PushFrameData(NewFrame);
}

bool UUEPopH264::Decoder::PushFrameData(FFrameInput Frame)
{
	if (!ThreadedDecoding)
	{
		int Length;
		if (Frame.Bytes.GetData() == NULL)
		{
			Length = 0;
		}
		else
		{
			Length = Frame.Bytes.Num();
		}
		int Result = popH264DLL.PopH264_PushData(Instance, Frame.Bytes.GetData(), Length, Frame.FrameNumber);
		//UE_LOG(LogTemp, Warning, TEXT("Result: %i"), Result);
	}

	//not doing threaded decoding yet
	return false;
}

int UUEPopH264::Decoder::GetNextFrame(TArray<UTexture2D*> Planes, TArray<PixelFormat> PixelFormats)
{
	FFrameMeta FrameMeta = GetNextFrameAndMeta(Planes, PixelFormats);
	return FrameMeta.FrameNumber;
	return 0;
}

FFrameMeta UUEPopH264::Decoder::GetNextFrameAndMeta(TArray<UTexture2D*> Planes, TArray<PixelFormat> PixelFormats)
{
	FFrameMeta Meta;
	char* JsonbufferAsChar = new char();// = TCHAR_TO_ANSI(*BytesToString(GetJsonBuffer().GetData(), GetJsonBuffer().Num()));
	int32_t JsonBufferSize = sizeof(JsonbufferAsChar);
	int32_t* MetaValues = new int32_t();

	//popH264DLL.PopH264_GetMeta(Instance, MetaValues, 4);
	//popH264DLL.PopH264_PeekFrame(Instance, JsonbufferAsChar, 444);
	//PopH264_PeekFrame(Instance, JsonbufferAsChar, 445);
	//popH264DLL.PopH264_EnumDecoders(JsonbufferAsChar, 1000);

	//returning up here to demonstrate that it's the popH264 calls that are the cause of my errors

	//FString Json = BytesToString(GetJsonBuffer().GetData(), GetJsonBuffer().Num());
	//TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<TCHAR>> Jsonread = TJsonReaderFactory<TCHAR>::Create(JsonbufferAsChar);
	FJsonObjectConverter::JsonObjectStringToUStruct(JsonbufferAsChar, &Meta, 0, 0);

	if (FJsonSerializer::Deserialize(Jsonread, JsonObject))
	{
		TArray <TSharedPtr<FJsonValue>> PlanesJs = JsonObject->GetArrayField("Planes");
		for (int i = 0; i < PlanesJs.Num(); i++)
		{
			FPlaneMeta Plane;
			TSharedPtr<FJsonObject> PlaneObj = PlanesJs[i]->AsObject();
			Plane.Channels = PlaneObj->GetIntegerField("Channels");
			Plane.DataSize = PlaneObj->GetIntegerField("DataSize");
			Plane.Format = PlaneObj->GetStringField("Format");
			Plane.Height = PlaneObj->GetIntegerField("Height");
			Plane.Width = PlaneObj->GetIntegerField("Width");
			Plane.PixelFormat = (PixelFormat)PlaneObj->GetIntegerField("PixelFormat");
			//FJsonObjectConverter::JsonObjectToUStruct(PlaneObj.ToSharedRef(), &Plane, 0, 0);
			Meta.Planes.Add(Plane);
		}
	}

	int PlaneCount = Meta.Planes.Num();

	if (Meta.EndOfStream)
		HadEndOfStream = true;

	if (PlaneCount <= 0)
	{
		PixelFormats.Empty();
		return FFrameMeta();
	}

	if (Meta.FrameNumber < 0)
	{
		return FFrameMeta();
	}

	for (int i = 0; i < PlaneCount && i < 4; i++)
	{
		Planes.Add(UTexture2D::CreateTransient(0, 0));
		PixelFormats.Add(PixelFormat());
		PlaneCaches.Add(TArray<uint8>());
	}
	//AllocListToSize(Planes, PlaneCount);
	//AllocListToSize(PixelFormats, PlaneCount);
	//AllocListToSize(PlaneCaches, PlaneCount);

	if (PlaneCount >= 1) PixelFormats[0] = Meta.Planes[0].PixelFormat;
	if (PlaneCount >= 2) PixelFormats[1] = Meta.Planes[1].PixelFormat;
	if (PlaneCount >= 3) PixelFormats[2] = Meta.Planes[2].PixelFormat;

	for (int p = 0; p < PlaneCount; p++)
	{
		if (PlaneCaches[p].GetData() != nullptr)
			continue;
		if (!Planes[p])
			continue;

		uint8* raw = NULL;
		FTexture2DMipMap& Mip = Planes[p]->PlatformData->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		raw = (uint8*)Data;

		TArray<uint8> RawAsArray;
		RawAsArray.Add(*raw);

		PlaneCaches[p] = RawAsArray;
	}

	auto Plane0Data = (PlaneCaches.Num() >= 1 && PlaneCaches[0].GetData() != nullptr) ? PlaneCaches[0] : UnusedBuffer;
	auto Plane1Data = (PlaneCaches.Num() >= 2 && PlaneCaches[1].GetData() != nullptr) ? PlaneCaches[1] : UnusedBuffer;
	auto Plane2Data = (PlaneCaches.Num() >= 3 && PlaneCaches[2].GetData() != nullptr) ? PlaneCaches[2] : UnusedBuffer;

	auto PopResult = popH264DLL.PopH264_PopFrame(Instance, Plane0Data.GetData(), Plane0Data.Num(), Plane1Data.GetData(), Plane1Data.Num(), Plane2Data.GetData(), Plane2Data.Num());
	if (PopResult < 0)
	{
		return FFrameMeta();
		UE_LOG(LogTemp, Error, TEXT("PopFrame Failed"));
	}

	for (int i = 0; i < PlaneCount; i++)
	{
		if (!Planes[i])
			continue;

	}
	Meta.FrameNumber = PopResult;
	return Meta;
}

TArray<uint8> UUEPopH264::Decoder::GetJsonBuffer()
{
	if (JsonBufferPrealloc.GetData() == nullptr)
	{
		JsonBufferPrealloc.Empty();
		JsonBufferPrealloc.AddDefaulted(1000);
	}

	return JsonBufferPrealloc;
}

UTexture2D* UUEPopH264::Decoder::AllocTexture(UTexture2D* Plane, FPlaneMeta Meta)
{
	auto Format = GetTextureFormat(Meta.Channels);

	if (Plane != nullptr)
		if (Plane->GetSurfaceWidth() != Meta.Width)
			return nullptr;
		else if (Plane->GetSurfaceHeight() != Meta.Height)
			return nullptr;
		else if (Plane->GetPixelFormat() != Format)
			return nullptr;

	if (!Plane)
	{
		bool MipMap = false;
		bool Liner = true;
		Plane = UTexture2D::CreateTransient(Meta.Width, Meta.Height, Format);
		Plane->Filter = TF_Default;
	}

	return Plane;
}

EPixelFormat UUEPopH264::Decoder::GetTextureFormat(int ComponentCount)
{
	//I might be getting the wrong values here but it seemed closest to the formats in Unity
	switch (ComponentCount)
	{
	case 1: return PF_R8;
	case 2: return PF_R8G8;
	case 3: return PF_FloatRGB;
	case 4: return PF_FloatRGBA;
	default:
		UE_LOG(LogTemp, Error, TEXT("Don't know what format to use for component count %i"), ComponentCount);
		return PF_Unknown;
	}
}
