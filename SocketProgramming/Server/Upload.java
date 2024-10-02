package Server;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;



public class Upload extends Thread{
    private Socket s;
    private DataInputStream in;
    private DataOutputStream out;
    private String privacy;
    private String fileName;
    private String src;
    private String dest;
    private long fileSize;
    private String userName;
    private int chunkSize;

    public Upload(Socket s, String userName, String fileName, String privacy, String src, String dest, long fileSize, int chunkSize) throws IOException {
        this.s=s;
        this.fileName=fileName;
        this.privacy=privacy;
        this.userName=userName;
        this.src=src;
        this.dest=dest;
        this.fileSize=fileSize;
        this.chunkSize=chunkSize;
        in = new DataInputStream(s.getInputStream());
        out = new DataOutputStream(s.getOutputStream());

    }

    @Override
    public void run(){
        try{
            int bytes = 0;
            FileOutputStream fileOutputStream = new FileOutputStream(fileName);
            long size = in.readLong();     // reading sizeof file
            long totalChunks = in.readLong();
            long receivedchunks = 0;
            byte[] buffer = new byte[chunkSize];
            /*
              IMPLEMENTATION DETAILS 3 & 4 (Now the client splits the file into chunks depending on the chunkSize, Upon receiving each chunk, the server sends an acknowledgement)
              dataInputStream.read(buffer, 0, length) is a method that reads data from the dataInputStream into the buffer.
              Math.min() It takes the minimum value between buffer.length (the length of the buffer) and size (the remaining size of the file) to ensure that only the available space in the buffer is read.
              Reads data from the input stream into the buffer. It returns the number of bytes read or -1 if the end of the stream is reached.
             */
            while (size > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, size))) != -1) {
                out.writeUTF("acknowledgement");
                out.flush();
                fileOutputStream.write(buffer,0,bytes);
                size -= bytes;      // read upto file size
                receivedchunks++;
            }
            fileOutputStream.close();
            if(totalChunks==receivedchunks){
                System.out.println(fileName+" received successfully");
                Files.move(Paths.get(src), Paths.get(dest));
                String []arr = fileName.split("_");
                //String userName = Server.socketUserHashMap1.get(s).getUserName();

                //  Iterating over the Server.requests list to find the corresponding request based on the filename.
                //If a match is found, it sends a message to the appropriate user who requested the file upload, notifying them of the successful upload
                for (Request request : Server.requests) {
                    if (request.getFilename().equalsIgnoreCase(arr[0])) {
                        Client user = request.getUser();
                        if (user != null) {
                            user.addMessage(userName + " has uploaded file " + arr[0]);
                            //Server.socketUserHashMap2.get(s).addMessage(userName + " has uploaded file " + arr[0]);
                           // System.out.println(userName + " has uploaded file " + arr[0]);
                            Client a;
                            for (Map.Entry<Socket, Client> entry : Server.socketUserHashMap2.entrySet()) {
                                Client client = entry.getValue();
                                Socket soc;
                                if (client.getUserName().equals(user)) {
                                    a = client;
                                    soc= entry.getKey();
                                    if(soc!=null)
                                    {
                                        System.out.println(userName + " has uploaded file " + arr[0]);
                                        Server.socketUserHashMap2.get(soc).addMessage(userName + " has uploaded file " + arr[0]);
                                    }
                                    else System.out.println("What?");
                                   // a.addMessage(userName + " has uploaded file " + arr[0]);
                                    break; // Stop iterating once the user is found
                                }
                            }
                            String m=user.checkMessages();
                            System.out.println(m);
                            System.out.println(user.getUserName());
                        }
                    }
                }
                /*for (Request request : Server.requests) {
                    if (request.getFilename().equalsIgnoreCase(arr[0])) {
                        for(Socket socket :Server.clientSockets2){
                            if(!socket.isClosed())
                            {
                                Client user = Server.socketUserHashMap2.get(socket);
                                if (user != null && user.getUserName().equalsIgnoreCase(request.getUser().getUserName())) {
                                    user.addMessage(userName + " has uploaded file " + arr[0]);
                                    System.out.println(userName + " has uploaded file " + arr[0]);
                                }
                            }
                        }
                    }
                }*/

            }else{
                System.out.println(fileName+" receiving failed");
                File file = new File(src);
                file.delete();
            }
            Server.AVAILABLE_BUFFER += fileSize;

        }catch (Exception e){
            System.out.println("Error in receiving the file");
            Server.AVAILABLE_BUFFER = (int) Math.min(Server.AVAILABLE_BUFFER+fileSize, Server.MAX_BUFFER_SIZE);
            e.printStackTrace();
        }
    }
}
