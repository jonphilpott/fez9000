local socket = require 'socket'

function love.load()
   host, port = "127.0.0.1", 31337
   tcp = assert(socket.tcp())
   tcp:connect(host, port)
   tcp:settimeout(0)
end

function love.update(dt)
   repeat
	  data, msg = tcp:receive()
	  if data then
		 print("msg received: ", data)
	  end
   until not data
end
