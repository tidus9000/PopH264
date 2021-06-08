// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ImportPopH264dll.h"
#include <popH264/popH264-1.3.38/include/popH264/PopH264.h>
#include "JsonObjectConverter.h"

#include "UEPopH264.generated.h"


// Sets default values for this component's properties

	//	gr: these numbers don't matter in PopH264, need a better way to map these across depedencies
	//		other than matching strings
	//	for use with PopYuv shader, these enum values should match the shader
enum PixelFormat
{
	Debug = 999,
	Invalid = 0,
	Greyscale = 1,
	RGB = 2,
	RGBA = 3,
	BGRA = 4,
	BGR = 5,
	YYuv_8888_Full = 6,
	YYuv_8888_Ntsc = 7,
	Depth16mm = 8,
	Chroma_U = 9,
	Chroma_V = 10,
	ChromaUV_88 = 11,
	ChromaVU_88 = 12,
	Luma_Ntsc = 13,


	ChromaU_8 = Chroma_U,
	ChromaV_8 = Chroma_V,
};

USTRUCT()
struct FPlaneMeta
{
	GENERATED_BODY()
public:
	PixelFormat		PixelFormat; // TODO: Must write some form of getter{ get { return (PixelFormat)Enum.Parse(typeof(PixelFormat), Format); } };
	FString			Format;
	int				Width;
	int				Height;
	int				DataSize;
	int				Channels;
};

USTRUCT()
struct FFrameMeta
{
	GENERATED_BODY()
		FFrameMeta() : FrameNumber(INT32_MAX) {}
public:
	UPROPERTY()
		TArray<FPlaneMeta>	Planes;
	UPROPERTY()
		FString			Error;
	UPROPERTY()
		FString			Decoder;				//	internal name of codec (if provided by API/OS)
	UPROPERTY()
		bool			HardwareAccelerated;	//	are we using a hardware accelerated decoder. DO NOT rely on this information as if not provided cs defaults to false. Currently MediaFoundation only
	UPROPERTY()
		bool			EndOfStream;
	UPROPERTY()
		int 			FrameNumber;
	UPROPERTY()
		int				QueuedFrames;	//	number of subsequent frames already decoded and buffered up

		// optional meta output by decoder
	UPROPERTY()
		int				Rotation;   //  clockwise rotation in degrees
	UPROPERTY()
		FString			YuvColourMatrixName;    //	todo: enum this
	UPROPERTY()
		int				AverageBitsPerSecondRate;
	UPROPERTY()
		int				RowStrideBytes;
	UPROPERTY()
		bool			Flipped;
	UPROPERTY()
		int				ImageWidth;
	UPROPERTY()
		int				ImageHeight;
	UPROPERTY()
		TArray<int>				ImageRect;		//	some decoders will output an image aligned to say, 16 (macro blocks, or byte alignment etc) If the image is padded, we should have a [x,y,w,h] array here
};

USTRUCT() struct FFrameInput
{
	GENERATED_BODY()
public:
	UPROPERTY()
		TArray<uint8> Bytes;
	UPROPERTY()
		int FrameNumber;

	bool GetEndOfStream()
	{
		return Bytes.GetData() == nullptr;
	}	//	marker/command to tell decoder stream has ended
};

USTRUCT()
struct FDecoderParams
{
	GENERATED_BODY()
		//	Avf, Broadway, MediaFoundation, MagicLeap, Intel etc
		//	empty string defaults to "best" (hardware where possible)
		//	todo: PopH264_EnumDecoders which will return a list of all possible decoders
		//	ie. low level specific decoders/codecs installed on the system, including say MediaFoundation_NvidiaHardwareH264, or MagicLeap_GoogleSoftware
		UPROPERTY()
		FString Decoder;

	//	print extra debug info (all decoders)
	UPROPERTY()
		bool VerboseDebug;

	UPROPERTY()
		bool AllowBuffering;			//	by default poph264 tries to reduce amount of buffering decoders do and deliver frames ASAP
	UPROPERTY()
		bool DoubleDecodeKeyframe;	//	Hack for broadway & MediaFoundation, process a keyframe twice to instantly decode instead of buffering
	UPROPERTY()
		bool DrainOnKeyframe;		//	Debug for MediaFoundation, trigger drain command on a keyfrae
	UPROPERTY()
		bool LowPowerMode;
	UPROPERTY()
		bool DropBadFrames;
	UPROPERTY()
		bool DecodeSei;

	//	gr: only set these for testing. 0 means no hint will be set
	//public int	Width;
	//public int	Height;
	//public int	InputSize;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POPH264_API UUEPopH264 : public UActorComponent
{
	GENERATED_BODY()
public:
	class Decoder
	{
	protected:
		ImportPopH264dll popH264DLL;
		int32_t Instance;

		TArray<TArray<uint8>> PlaneCaches;
		TArray<uint8> UnusedBuffer;
		bool ThreadedDecoding = true;
		TArray<FFrameInput> InputQueue;
		bool InputThreadResult;
		bool HadEndOfStream = false;

		TArray<uint8> JsonBufferPrealloc;
		TArray<uint8> GetJsonBuffer();

	public:
		Decoder(FDecoderParams _DecoderParams, bool _ThreadedDecoding);
		~Decoder();
		void Dispose();
		UTexture2D* AllocTexture(UTexture2D* Plane, FPlaneMeta Meta);
		bool PushFrameData(TArray<uint8> H264Data, int FrameNumber);
		bool PushFrameData(FFrameInput Frame);
		FFrameMeta GetNextFrameAndMeta(TArray<UTexture2D*> Planes, TArray<PixelFormat> PixelFormats);
		int  GetNextFrame(TArray<UTexture2D*> Planes, TArray<PixelFormat> PixelFormats);

	protected:
		template <typename T>
		void AllocListToSize(TArray<T> Array, int size);
		EPixelFormat GetTextureFormat(int ComponentCount);
	};

	UUEPopH264();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



		
};

template<typename T>
inline void UUEPopH264::Decoder::AllocListToSize(TArray<T> Array, int size)
{
	while (Array.Num() < size)
	{
		Array.Add(T());
	}
}