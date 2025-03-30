import com.sun.jna.platform.win32.Kernel32
import com.sun.jna.platform.win32.WinNT
import com.sun.jna.ptr.IntByReference
import java.io.IOException

class Pipe {
    companion object {
        const val PIPE_NAME = "\\\\.\\pipe\\OmerPipe"
    }

    private var pipeHandle: WinNT.HANDLE? = null

    fun pipeInit() {
        try {
            //starting the pipe handle
            pipeHandle = Kernel32.INSTANCE.CreateFile(
                PIPE_NAME,
                WinNT.GENERIC_READ or WinNT.GENERIC_WRITE,
                0,
                null,
                WinNT.OPEN_EXISTING,
                WinNT.FILE_FLAG_OVERLAPPED,
                null
            )
            //
            if (pipeHandle == WinNT.INVALID_HANDLE_VALUE) {
                throw IOException("failed connecting to pipe")
            }

            println("Successfully connected to pipe!")
        } catch (e: Exception) {
            println("Pipe connection error: ${e.message}")
            e.printStackTrace()
        }
    }

    fun sendMessage(message: String) {
        try {
            //checking we have pipe
            if (pipeHandle == null || pipeHandle == WinNT.INVALID_HANDLE_VALUE) {
                pipeInit()
            }

            // making message
            val messageBytes = message.toByteArray()
            val writtenBytes = IntByReference(0)

            val writeSuccess = Kernel32.INSTANCE.WriteFile(
                pipeHandle,
                messageBytes,
                messageBytes.size,
                writtenBytes,
                null
            )
            //if failed
            if (!writeSuccess) {
                println("failed message")
            } else {
                println("Message Sent: $message")
            }
        } catch (e: Exception) {
            println("Error sending message: ${e.message}")
            e.printStackTrace()
        }
    }

    fun readMessage(): String? {
        try {
            //checking pipe
            if (pipeHandle == null || pipeHandle == WinNT.INVALID_HANDLE_VALUE) {
                pipeInit()
            }
            //bytes for reading and peeking so we dont block the connections
            val bytesAvailable = IntByReference(0)
            if (Kernel32.INSTANCE.PeekNamedPipe(
                    pipeHandle,
                    null,
                    0,
                    null,
                    bytesAvailable,
                    null
                ) && bytesAvailable.value > 0
            ) {
                val buffer = ByteArray(bytesAvailable.value)
                val bytesRead = IntByReference(0)

                val readSuccess = Kernel32.INSTANCE.ReadFile(
                    pipeHandle,
                    buffer,
                    buffer.size,
                    bytesRead,
                    null
                )

                return if (readSuccess) {
                    String(buffer, 0, bytesRead.value).trim()
                } else {
                    println("failed to read from pipe")
                    null
                }
            }

            return null
        } catch (e: Exception) {
            println("Error reading message: ${e.message}")
            e.printStackTrace()
            return null
        }
    }
    //closing the pipe
    fun close() {
        try {
            //closing the pipe
            pipeHandle?.let {
                Kernel32.INSTANCE.CloseHandle(it)
            }

            pipeHandle = null
            println("Pipe closed successfully")
        } catch (e: Exception) {
            println("Error closing pipe: ${e.message}")
            e.printStackTrace()
        }
    }
}