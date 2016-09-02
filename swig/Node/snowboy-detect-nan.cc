// Copyright 2016  KITT.AI (author: Guoguo Chen)
// Node module (author: Evan Cohen)

#include <nan.h>
#include <cassert>
#include <csignal>
#include <iostream>
#include <pa_ringbuffer.h>
#include <pa_util.h>
#include <portaudio.h>
#include <string>
#include <vector>

#include "include/snowboy-detect.h"

// OLD CODE

int PortAudioCallback(const void* input,
                      void* output,
                      unsigned long frame_count,
                      const PaStreamCallbackTimeInfo* time_info,
                      PaStreamCallbackFlags status_flags,
                      void* user_data);

class PortAudioWrapper {
 public:
  // Constructor.
  PortAudioWrapper(int sample_rate, int num_channels, int bits_per_sample) {
    num_lost_samples_ = 0;
    min_read_samples_ = sample_rate * 0.1;
    Init(sample_rate, num_channels, bits_per_sample);
  }

  // Reads data from ring buffer.
  template<typename T>
  void Read(std::vector<T>* data) {
    assert(data != NULL);

    // Checks ring buffer overflow.
    if (num_lost_samples_ > 0) {
      std::cerr << "Lost " << num_lost_samples_ << " samples due to ring"
          << " buffer overflow." << std::endl;
      num_lost_samples_ = 0;
    }

    ring_buffer_size_t num_available_samples = 0;
    while (true) {
      num_available_samples =
          PaUtil_GetRingBufferReadAvailable(&pa_ringbuffer_);
      if (num_available_samples >= min_read_samples_) {
        break;
      }
      Pa_Sleep(5);
    }

    // Reads data.
    num_available_samples = PaUtil_GetRingBufferReadAvailable(&pa_ringbuffer_);
    data->resize(num_available_samples);
    ring_buffer_size_t num_read_samples = PaUtil_ReadRingBuffer(
        &pa_ringbuffer_, data->data(), num_available_samples);
    if (num_read_samples != num_available_samples) {
      std::cerr << num_available_samples << " samples were available,  but "
          << "only " << num_read_samples << " samples were read." << std::endl;
    }
  }

  int Callback(const void* input, void* output,
               unsigned long frame_count,
               const PaStreamCallbackTimeInfo* time_info,
               PaStreamCallbackFlags status_flags) {
    // Input audio.
    ring_buffer_size_t num_written_samples =
        PaUtil_WriteRingBuffer(&pa_ringbuffer_, input, frame_count);
    num_lost_samples_ += frame_count - num_written_samples;
    return paContinue;
  }

  ~PortAudioWrapper() {
    Pa_StopStream(pa_stream_);
    Pa_CloseStream(pa_stream_);
    Pa_Terminate();
    PaUtil_FreeMemory(ringbuffer_);
  }

 private:
  // Initialization.
  bool Init(int sample_rate, int num_channels, int bits_per_sample) {
    // Allocates ring buffer memory.
    int ringbuffer_size = 16384;
    ringbuffer_ = static_cast<char*>(
        PaUtil_AllocateMemory(bits_per_sample / 8 * ringbuffer_size));
    if (ringbuffer_ == NULL) {
      std::cerr << "Fail to allocate memory for ring buffer." << std::endl;
      return false;
    }

    // Initializes PortAudio ring buffer.
    ring_buffer_size_t rb_init_ans =
        PaUtil_InitializeRingBuffer(&pa_ringbuffer_, bits_per_sample / 8,
                                    ringbuffer_size, ringbuffer_);
    if (rb_init_ans == -1) {
      std::cerr << "Ring buffer size is not power of 2." << std::endl;
      return false;
    }

    // Initializes PortAudio.
    PaError pa_init_ans = Pa_Initialize();
    if (pa_init_ans != paNoError) {
      std::cerr << "Fail to initialize PortAudio, error message is \""
          << Pa_GetErrorText(pa_init_ans) << "\"" << std::endl;
      return false;
    }

    PaError pa_open_ans;
    if (bits_per_sample == 8) {
      pa_open_ans = Pa_OpenDefaultStream(
          &pa_stream_, num_channels, 0, paUInt8, sample_rate,
          paFramesPerBufferUnspecified, PortAudioCallback, this);
    } else if (bits_per_sample == 16) {
      pa_open_ans = Pa_OpenDefaultStream(
          &pa_stream_, num_channels, 0, paInt16, sample_rate,
          paFramesPerBufferUnspecified, PortAudioCallback, this);
    } else if (bits_per_sample == 32) {
      pa_open_ans = Pa_OpenDefaultStream(
          &pa_stream_, num_channels, 0, paInt32, sample_rate,
          paFramesPerBufferUnspecified, PortAudioCallback, this);
    } else {
      std::cerr << "Unsupported BitsPerSample: " << bits_per_sample
          << std::endl;
      return false;
    }
    if (pa_open_ans != paNoError) {
      std::cerr << "Fail to open PortAudio stream, error message is \""
          << Pa_GetErrorText(pa_open_ans) << "\"" << std::endl;
      return false;
    }

    PaError pa_stream_start_ans = Pa_StartStream(pa_stream_);
    if (pa_stream_start_ans != paNoError) {
      std::cerr << "Fail to start PortAudio stream, error message is \""
          << Pa_GetErrorText(pa_stream_start_ans) << "\"" << std::endl;
      return false;
    }
    return true;
  }

 private:
  // Pointer to the ring buffer memory.
  char* ringbuffer_;

  // Ring buffer wrapper used in PortAudio.
  PaUtilRingBuffer pa_ringbuffer_;

  // Pointer to PortAudio stream.
  PaStream* pa_stream_;

  // Number of lost samples at each Read() due to ring buffer overflow.
  int num_lost_samples_;

  // Wait for this number of samples in each Read() call.
  int min_read_samples_;
};

int PortAudioCallback(const void* input,
                      void* output,
                      unsigned long frame_count,
                      const PaStreamCallbackTimeInfo* time_info,
                      PaStreamCallbackFlags status_flags,
                      void* user_data) {
  PortAudioWrapper* pa_wrapper = reinterpret_cast<PortAudioWrapper*>(user_data);
  pa_wrapper->Callback(input, output, frame_count, time_info, status_flags);
  return paContinue;
}

void SignalHandler(int signal){
  std::cerr << "Caught signal " << signal << ", terminating..." << std::endl;
  exit(0);
}


// While the module is recognizing
bool recognizing = false;

void Detect(const Nan::FunctionCallbackInfo<v8::Value>& info) {

  if (info.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!info[0]->IsString() || !info[1]->IsString() || !info[2]->IsFunction()) {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  // Comma seperated strings with models, ex: "resources/snowboy.umdl,resources/alexa.pmdl"
  v8::String::Utf8Value param1(info[0]->ToString());
  std::string model_filename = std::string(*param1);

  // Comma seperated strings with sensitivities, ex "0.4,0.4"
  v8::String::Utf8Value param2(info[1]->ToString());
  std::string sensitivity_str = std::string(*param2);

  // Callback function
  v8::Local<v8::Function> callbackHandle = info[2].As<v8::Function>();

  // Common resource file for umdl model
  std::string resource_filename = "../../resources/common.res";

  // Configures signal handling.
  struct sigaction sig_int_handler;
   sig_int_handler.sa_handler = SignalHandler;
   sigemptyset(&sig_int_handler.sa_mask);
   sig_int_handler.sa_flags = 0;
   sigaction(SIGINT, &sig_int_handler, NULL);

   float audio_gain = 1;

  // Initializes Snowboy detector.
  snowboy::SnowboyDetect detector(resource_filename, model_filename);
  detector.SetSensitivity(sensitivity_str);
  detector.SetAudioGain(audio_gain);

  // Initializes PortAudio. You may use other tools to capture the audio.
  PortAudioWrapper pa_wrapper(detector.SampleRate(),
                              detector.NumChannels(), detector.BitsPerSample());

  // Runs the detection.
  // Note: I hard-coded <int16_t> as data type because detector.BitsPerSample()
  //       returns 16.
  std::cout << "Listening... Press Ctrl+C to exit" << std::endl;
  std::vector<int16_t> data;

  recognizing = true;
  while (recognizing) {
    pa_wrapper.Read(&data);
    if (data.size() != 0) {
      int result = detector.RunDetection(data.data(), data.size());
      // Make callback
      v8::Local<v8::Value> argv[] = {
        Nan::New<v8::Number>(result)
      };
      Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callbackHandle, 1, argv);
    }
  }
}

class DetectWorker : public Nan::AsyncProgressWorker {
 public:
  DetectWorker(
      Nan::Callback *callback
    , Nan::Callback *progress
    , std::string model_filename
    , std::string sensitivity_str)
    : AsyncProgressWorkerBase(callback), progress(progress)
    , model_filename(model_filename), sensitivity_str(sensitivity_str) {}
  ~DetectWorker() {}

  void Execute (const Nan::AsyncProgressWorker::ExecutionProgress& progress) {
    // Common resource file for umdl model
    std::string resource_filename = "../../resources/common.res";

    // Configures signal handling.
    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = SignalHandler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    sigaction(SIGINT, &sig_int_handler, NULL);

    float audio_gain = 1;

    // Initializes Snowboy detector.
    snowboy::SnowboyDetect detector(resource_filename, model_filename);
    detector.SetSensitivity(sensitivity_str);
    detector.SetAudioGain(audio_gain);

    // Initializes PortAudio. You may use other tools to capture the audio.
    PortAudioWrapper pa_wrapper(detector.SampleRate(),
                                detector.NumChannels(), detector.BitsPerSample());

    // Runs the detection.
    // Note: I hard-coded <int16_t> as data type because detector.BitsPerSample()
    //       returns 16.
    std::cout << "Listening... Press Ctrl+C to exit" << std::endl;
    std::vector<int16_t> data;

    recognizing = true;
    while (recognizing) {
      pa_wrapper.Read(&data);
      if (data.size() != 0) {
        int result = detector.RunDetection(data.data(), data.size());
        // Make callback
        // v8::Local<v8::Value> argv[] = {
        //   Nan::New<v8::Number>(result)
        // };
        // Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callbackHandle, 1, argv);
        progress.Send(reinterpret_cast<const char*>(&result), sizeof(int));
      }
    }


    // for (int i = 0; i < 100; ++i) {
    //   progress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
    //   Sleep(100);
    // }
  }

  void HandleProgressCallback(const char *data, size_t size) {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::New<v8::Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
    };
    progress->Call(1, argv);
  }

 private:
  Nan::Callback *progress;
  std::string model_filename;
  std::string sensitivity_str;
};

NAN_METHOD(DoProgress) {
  Nan::Callback *progress = new Nan::Callback(info[2].As<v8::Function>());
  Nan::Callback *callback = new Nan::Callback(info[3].As<v8::Function>());

  v8::String::Utf8Value param1(info[0]->ToString());
  std::string model_filename = std::string(*param1);

  v8::String::Utf8Value param2(info[1]->ToString());
  std::string sensitivity_str = std::string(*param2);

  AsyncQueueWorker(new DetectWorker(
      callback
    , progress
    , model_filename
    , sensitivity_str));
}

NAN_METHOD(IsListening) {
  info.GetReturnValue().Set(recognizing);
}

NAN_METHOD(Stop) {
  recognizing = false;
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("detect").ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(Detect)->GetFunction());
  exports->Set(Nan::New("isListening").ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(IsListening)->GetFunction());
  exports->Set(Nan::New("stop").ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(Stop)->GetFunction());
  exports->Set(Nan::New("doProgress").ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(DoProgress)->GetFunction());
}

NODE_MODULE(addon, Init)