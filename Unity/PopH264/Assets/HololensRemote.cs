﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;


public class HololensRemote : FileReaderBase
{
	public string HttpUsername = "admin";
	public string HttpPassword = "password";
	public string MonitorUrl = "https://192.168.0.36/api/holographic/stream/live_low.mp4?holo=true&pv=true&mic=false&loopback=false";
	public UnityEvent_String OnError;
	public int LoadMp4AfterKb = 1024 * 2;
	public int LoadMp4AfterBytes { get { return LoadMp4AfterKb * 1024; } }

	List<byte> PendingBytes;
	long BytesRead = 0;

	void OnEnable()
	{
		LoadStream();
	}

	public override long GetKnownFileSize()
	{
		var Size = BytesRead;
		if (PendingBytes != null)
			Size += PendingBytes.Count;
		return Size;
	}


	override public System.Func<long, long, byte[]> GetReadFileFunction()
	{
		return ReadFileBytes;
	}

	byte[] ReadFileBytes(long Position, long Size)
	{
		//	not yet read any data, use of GetKnownFileSize() should have caught this
		if (PendingBytes == null)
			return null;

		var Data = new byte[Size];
		PendingBytes.CopyTo(0, Data, 0, (int)Size);

		//	delete data read so we don't run out of memory
		PendingBytes.RemoveRange(0, (int)Size);
		BytesRead -= Size;

		return Data;
	}

	public void LoadStream()
	{
		var Mp4 = GetComponent<Mp4>();

		/*
		//	download mp4 from stream
		var Track = Stream.GetVideoTrack();
		var Source = Track.Sources[0];
		var ChunkQueueUrls = new List<string>();
		for (var ChunkIndex = 0; ChunkIndex < Track.ChunkCount; ChunkIndex++)
		{
			var ChunkUrl = Track.GetUrl(Source.BitRate, ChunkIndex, Stream.BaseUrl);
			ChunkQueueUrls.Add(ChunkUrl);
		}

		//	todo: convert to bytes here and remove from Mp4.cs
		var TrackSpsAndPps = Source.CodecData_Hex;
		*/
		System.Action<string> HandleError = (Error) =>
		{
			Debug.LogError(Error);
			OnError.Invoke(Error);
		};

		System.Action<byte[]> HandleMp4Bytes = (Bytes) =>
		{
			if (PendingBytes == null)
				PendingBytes = new List<byte>();
			PendingBytes.AddRange(Bytes);
			Mp4.enabled = true;
		};

		string authorization = Authenticate(HttpUsername, HttpPassword);

		StartCoroutine(LoadMp4(MonitorUrl, authorization, LoadMp4AfterBytes, HandleError, HandleMp4Bytes));
	}

    //  SSL isn't going to work on 2017
#if UNITY_2018_OR_NEWER
    class Cert : CertificateHandler
	{
		protected override bool ValidateCertificate(byte[] certificateData)
		{
			return true;
		}

	};
#endif

    string Authenticate(string username, string password)
	{
		string auth = username + ":" + password;
		auth = System.Convert.ToBase64String(System.Text.Encoding.GetEncoding("ISO-8859-1").GetBytes(auth));
		auth = "Basic " + auth;
		return auth;
	}

	public class DownloadStreamer : DownloadHandlerScript
	{
		public int contentLength { get { return _received > _contentLength ? _received : _contentLength; } }

		private int _contentLength;
		private int _received;
		System.Action<byte[]> OnChunk;

		public DownloadStreamer(System.Action<byte[]> OnChunk,int BufferSize=4096) : base(new byte[BufferSize])
		{
			this.OnChunk = OnChunk;
		}

		protected override float GetProgress()
		{
			return contentLength <= 0 ? 0 : Mathf.Clamp01((float)_received / (float)contentLength);
		}

		protected override void ReceiveContentLength(int contentLength)
		{
			_contentLength = contentLength;
		}

		protected override bool ReceiveData(byte[] data, int dataLength)
		{
			if (data == null || data.Length == 0) return false;

			_received += dataLength;
			if (dataLength != data.Length)
			{
				var Chunk = new byte[dataLength];
				System.Array.Copy(data, Chunk, dataLength);
				OnChunk.Invoke(Chunk);
			}
			else
			{
				OnChunk.Invoke(data);
			}
			return true;
		}

		protected override void CompleteContent()
		{
			CloseStream();
		}

		public new void Dispose()
		{
			CloseStream();
			base.Dispose();
		}

		private void CloseStream()
		{

		}
	}


	static IEnumerator LoadMp4(string Url,string HttpAuth,int PushAfterXBytes,System.Action<string> OnError, System.Action<byte[]> OnBytesDownloaded)
	{
		var Mp4Bytes = new List<byte>();
		bool Done = false;
		System.Action<byte[]> OnDownloadedChunk = (Bytes) =>
		{
			//	todo: split at NAL packet, there is no header! but try and get content type
			Debug.Log("downloaded " + Bytes.Length + " bytes...");
			Mp4Bytes.AddRange(Bytes);

			if (Mp4Bytes.Count > PushAfterXBytes && !Done)
			{
				Done = true;
				OnBytesDownloaded.Invoke(Mp4Bytes.ToArray());
			}
		};

		{
			Debug.Log("Fetching " + Url);
			var www = UnityWebRequest.Get(Url);
#if UNITY_2018_OR_NEWER
			www.certificateHandler = new Cert();
#endif
			www.downloadHandler = new DownloadStreamer(OnDownloadedChunk);

			if(HttpAuth != null )
				www.SetRequestHeader("AUTHORIZATION", HttpAuth);

			yield return www.SendWebRequest();

			if (www.isNetworkError || www.isHttpError)
			{
				OnError(Url + " error: " + www.error + " response: " + www.responseCode);
				yield break;
			}

			//	Show results as text
			Debug.Log(www.downloadHandler.text);
			try
			{
				var Bytes = www.downloadHandler.data;
				OnBytesDownloaded(Bytes);
			}
			catch (System.Exception e)
			{
				OnError(e.Message);
			}
		}
	}
}
