/// <reference types="dom-webcodecs" />

const pendingFrames = []; // 도착하는 & 처리되지 않은 프레임을 담을 배열 Array to store decoded video frames
let underflow = true; // 다음 프레임을 처리할 준비가 되었는지 확인하는 flag / Boolean flag to check if there's a frame underflow
let time_base = 0; // base time 초기화 base time for time calculation
let startTime = 0.0; // 시작 시간 초기화 Start time for video rendering
let isKeyFrameRecv = false; // 키 프레임이 수신되었는지 확인하는 flag / Flag to check if keyframe has been received

let decoder; // 비디오 디코더 인스턴스 VideoDecoder instance
let canvas

// 수신된 프레임을 처리 Callback function to handle decoded frames
const handleFrame = frame => {
  pendingFrames.push(frame); // 수신된 프레임을 보류 중인 프레임 배열로 푸시 Push decoded frame to the pendingFrames array
  if (underflow) setTimeout(renderFrame, 0); // 언더플로가 있는 경우 renderFrame 함수를 호출 If there's a frame underflow, set timeout to render the next frame
};

// 비디오의 MIME 유형에 대한 데이터 Video mime data
let mimeData = {
  codec: '',
  width: 0,
  height: 0,
  bitrate: 0,
  hardwareAcceleration: 'prefer-hardware',
  framerate: 30,
};

// VideoDecoder 인스턴스의 초기화 옵션 Configuration object for VideoDecoder instance
const init = {
  output: handleFrame, // 프레임 처리 Set handleFrame as the output callback
  error: e => {
    console.log(e); // 에러 메시지 출력 Set error callback to log error messages
  },
};

// 비디오 디코더를 시작 Function to start video decoding
const configVideoDecoder = async (mime, canvasElement) => {
  canvas = canvasElement;
  // MIME 데이터 업데이트 Update mimeData with the given mime type
  mimeData = mime;
  try {
    // 코덱 구성이 지원되는지 확인 Check if the given mime type is supported
    const { supported } = await VideoEncoder.isConfigSupported(mime);

    // 지원되는 경우 새 VideoDecoder 인스턴스를 만들고 MIME 데이터로 구성
    // If supported, create a VideoDecoder instance and configure it with the given mime type
    if (supported) {
      decoder = new VideoDecoder(init);
      decoder.configure(mime);
      // MIME 데이터로 구성된 후 디코더가 구성되지 않았는지 확인
      // If mime type is provided and the decoder state is unconfigured, log a message
      if (mime && decoder.state == 'unconfigured') {
        console.log('decoder reconfig');
      }
    } else {
      // 지원되지 않는 경우 경고 표시
      // If mime type is not supported, log an error message and show an alert
      console.log('Codec Configuration Not supported');
      alert('codec을 지원하지 않습니다.');
      return;
    }
  } catch (error) {
    console.log('VideoEncoder Error : ', error);
  }
};

// 비디오에서 프레임을 디코딩 Function to decode a video frame
const decodeVideoFrame = async (evtByte) => {
  // 메시지 이벤트 시 WebSocket에서 프레임 데이터 수신 Create an EncodedVideoChunk instance from the received data
  const chunk = new EncodedVideoChunk({
    timestamp: evtByte.timeStamp, // 프레임의 타임스탬프
    type: evtByte.type ? 'key' : 'delta', // 프레임 유형(키 또는 델타)
    data: new Uint8Array(evtByte.data), // Uint8 어레이로서의 프레임 데이터
    duration: undefined, // 프레임의 지속 시간
  });

  if (chunk.type === 'delta') {
    // 수신된 chunk가 키 프레임이 아니고 아직 키 프레임이 수신되지 않은 경우 즉시 반환
    // If the received chunk is not a key frame and no key frame has been received yet, return immediately
    if (!isKeyFrameRecv) return;
  } else {
    // 키 프레임이 수신되었음을 나타내는 플래그 설정 Set the flag to indicate that a key frame has been received
    isKeyFrameRecv = true;
  }

  try {
    // decoder 사용하여 chunk 데이터 decoding Decode the chunk data using the decoder
    return decoder.decode(chunk);
  } catch (err) {
    console.log(err); // chunk 데이터를 decoding하는 동안 발생하는 오류 기록 Log any error that occurs while decoding the chunk data
  }
};

const calculateTimeUntilNextFrame = timestamp => {
  // time_base가 설정되지 않은 경우 현재 성능 시간으로 설정 If the time_base has not been set, set it to the current performance time
  if (time_base == 0) time_base = performance.now();
  // 미디어 시간을 현재 성능 시간과 time_base의 차이로 계산 Calculate the media time as the difference between the current performance time and the time_base
  const media_time = performance.now() - time_base;
  // 다음 프레임이 렌더링될 때까지의 시간을 반환 Return the amount of time until the next frame should be rendered
  return Math.max(0, timestamp / 1000 - media_time);
};

const renderFrame = async () => {
  // 시작 시간이 설정되지 않은 경우 현재 수행 시간으로 설정 If the start time has not been set, set it to the current performance time
  if (startTime === 0.0) {
    startTime = performance.now();
  }

  // 보류 중인 프레임이 없는 경우 언더플로를 true로 설정 Set underflow to true if there are no pending frames
  underflow = pendingFrames.length == 0;
  if (underflow) return;

  // 보류 중인 프레임 배열에서 다음 프레임 제거 Remove the next frame from the pendingFrames array
  const frame = pendingFrames.shift();

  // 다음 프레임이 렌더링될 때까지의 시간 계산 Calculate the amount of time until the next frame should be rendered
  const timeUntilNextFrame = calculateTimeUntilNextFrame(frame.timestamp);
  // 다음 프레임이 렌더링될 때까지 대기 시간 Wait for the amount of time until the next frame should be rendered
  await new Promise(r => {
    setTimeout(r, timeUntilNextFrame);
  });

  // decoding된 frame 전달 Notify the decoder observer that a new chunk of video data has been decoded
  // const decoderObserver = Observer.getInstance()
  // decoderObserver.notifyObserver("decodeVideo_frame", frame)
  if (canvas) {
    canvas.width = mimeData.width;
    canvas.height = mimeData.height;

    const ctx = canvas.getContext('2d');

    // console.log(canvas.height);
    // console.log(canvas.width);

    // var aspectRatio = frame.width / frame.height;

    // // Set the canvas size based on the aspect ratio
    // if (aspectRatio >= canvas.width / canvas.height) {
    //   // The video frame is wider than the canvas
    //   canvas.height = canvas.width / aspectRatio;
    // } else {
    //   // The video frame is taller than the canvas
    //   canvas.width = canvas.height * aspectRatio;
    // }

    // console.log(aspectRatio)
    // console.log(canvas.height);
    // console.log(canvas.width);

    ctx.drawImage(frame, 0, 0);// , canvas.width, canvas.height
  } else {
    console.log("There's no canvas.")
  }

  // 프레임을 닫고 다음 렌더프레임 호출을 예약 Close the frame and schedule the next renderFrame call
  frame.close();
  setTimeout(renderFrame, 0);
};

const stopVideoDecode = async () => {
  try {
    // 디코더를 플러시하고 닫음 Flush the decoder and close it
    await decoder.flush();
    decoder.close();
  } catch (err) {
    // Video decoding을 중지하는 동안 발생하는 오류 기록 Log any error that occurs while stopping the video decode
    console.log('stopVideoDecode err : ', err);
  }
};
