#ifndef MARS_STREAM_CONTROLLER_H
#define MARS_STREAM_CONTROLLER_H

#include <string>

namespace net {

enum StreamType {
  STREAM_DATA,
  STREAM_AUDIO,
  STREAM_VIDEO
};

class StreamSenderClient {
 public:
  virtual void OnComplete(StreamType type, int stream_id, size_t sent_size) = 0;
  virtual void OnJam() = 0;
};

class StreamReceiverClient {
 public:
  virtual void OnReceive(StreamType type, const int peer_id, int stream_id, size_t received_size) = 0;
  virtual void OnNetworkScore(int score) = 0;
};

class StreamController {
 public:
  /* Create
   *
   * DESCRIPTION:
   *  A static factory method to create an instance of StreamController interface.
   *
   * PARAMETERS:
   *  client
   *   An instance of StreamSenderClient interface.
   *
   * RETURN_VALUE:
   *  A new instance of StreamController interface.
   */
  static StreamController* Create(StreamSenderClient* client);

  /* Start
   *
   * DESCRIPTION:
   *  Establish a UDP service port and start to read. Even if you are willing to
   *  send a stream data to other participant, this should be run before to call
   *  Send();
   *
   * PARAMETERS:
   *  N/A
   *
   * RETURN_VALUE:
   *  N/A
   */
  virtual void Start() = 0;

  /* Stop
   *
   * DESCRIPTION:
   *  Stop a UDP service port and stop to read. After this call, Send() will not send
   *  stream data to other participant. But Receive() might work if there're already
   *  received but not yet fetched stream data from other participant.
   *
   * PARAMETERS:
   *  N/A
   *
   * RETURN_VALUE:
   *  N/A
   */
  virtual void Stop() = 0;

  /* Shutdown
   *
   * DESCRIPTION:
   *  Quite similar to Stop(). but this will remove every participant from management.
   *
   * PARAMETERS:
   *  N/A
   *
   * RETURN_VALUE:
   *  N/A
   */
  virtual void Shutdown() = 0;


  /* AddPeer
   *
   * DESCRIPTION:
   *  Adds a v/c participant to talk with.
   *
   * PARAMETERS:
   *  ip_address
   *   An ip address of a participant
   *  client
   *   A pointer to Stream receiver interface
   *  
   * RETURN_VALUE:
   *  An unique id of the participant, peer_id. You should give this id when you retrieve
   *  stream data from the participant. See Receive()
   */
  virtual const int AddPeer(const std::string& ip_address, StreamReceiverClient* client) = 0;

  /* RemovePeer
   *
   * DESCRIPTION:
   *  Remove a v/c participant to talk with. No longer stream data will be received even though
   *  the participant send to us.
   *
   * PARAMETERS:
   *  peer_id 
   *   An unique id of the participant which is returned from AddPeer().
   *
   * RETURN_VALUE:
   *  N/A
   */
  virtual void RemovePeer(const int peer_id) = 0;

  
  /* Send
   *
   * DESCRIPTION:
   *  Send stream to every participants in the conference.
   *
   * PARAMETERS:
   *  type
   *   Stream type to deliver. It shall be one of either STREAM_VIDEO or STREAM_AUDIO.
   *  buf
   *   A unit of stream. The referenced data or memory should be valid at least in this
   *   call.
   *  buflen
   *   The length of stream data to send.
   *
   * RETURN_VALUE:
   *  The unique id of given stream.
   */
  virtual int Send(StreamType type, char* buf, size_t buflen, bool guaranteed = false) = 0;

  /* Receive
   *
   * DESCRIPTION:
   *  Receive stream from a participant in the conference.
   *
   * PARAMETERS:
   *  peer_id
   *   The id of participant who sent the stream data. You would be given in AddPeer()
   *   and StreamReceiverClient::OnReceived().
   *  stream_id
   *   The id of unit of stream, which was given in StreamReceiverClient::OnReceived().
   *  buf
   *   A buffer to copy stream data. The size of buffer SHOULD be equal to or larger than
   *   the length given in StreamReceiverClient::OnReceived().
   *  buflen
   *   The size of buf.
   *
   * RETURN_VALUE:
   *  The length of copied stream data.
   */
  virtual size_t Receive(const int peer_id, int stream_id, char* buf, size_t buflen) = 0;
};

} // namespace net

#endif // MARS_STREAM_CONTROLLER_H
