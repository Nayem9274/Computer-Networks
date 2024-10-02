package Client;


import Server.Server;

import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Upload extends Thread {

    private Socket s;
    private DataInputStream in;
    private DataOutputStream out;
    private String privacy;
    private String path;
    private long fileSize;
    private int chunkSize;

    public Upload(Socket s, String path, String privacy, long fileSize, int chunkSize) throws IOException {
        this.s = s;
        this.privacy = privacy;
        this.path = path;
        this.fileSize = fileSize;
        this.chunkSize = chunkSize;
        in = new DataInputStream(s.getInputStream());
        out = new DataOutputStream(s.getOutputStream());

    }

    public void run() {
        try {
            int bytes = 0;

            String p="C:\\Users\\Asus\\Desktop\\ABDULLAH WASI\\3-2\\CSE 321 322\\Lab\\Assignment_1\\"+path;
            File file = new File(p);
            file.createNewFile();
            FileInputStream fileInputStream = new FileInputStream("C:\\Users\\Asus\\Desktop\\ABDULLAH WASI\\3-2\\CSE 321 322\\Lab\\Assignment_1\\"+path);
            // sending file size
            out.writeLong(file.length());
            out.flush();
            // calculating the total number of chunks based on the chunk size and sends it to the receiver.
            long size = file.length();
            long totalChunks = size / chunkSize;
            long receivedchunks = 0;
            if (size % chunkSize != 0) {
                totalChunks++;
            }
            out.writeLong(totalChunks);
            out.flush();
            byte[] buffer = new byte[chunkSize];
            /*
            Iteratively reading chunks from the file, sending each chunk, and waiting for an acknowledgement (receivedchunks) from the receiver.
            If an acknowledgement is received, the receivedchunk counter is incremented.
            The process continues until all chunks have been sent and acknowledged.
            After sending the entire file, the input stream is closed
             */
            while ((bytes=fileInputStream.read(buffer))!=-1){
                if(receivedchunks == 0){
                    receivedchunks++;
                }else{
                    s.setSoTimeout(30000); // If the client does not receive any acknowledgement within 30 seconds, it sends a timeout message to the server and terminates the transmission.
                    try{
                        if(in.readUTF().equalsIgnoreCase("acknowledgement")){
                            receivedchunks++;
                        }else {
                            System.out.println("Chunk not sent");
                            return;
                        }
                    }catch (Exception e){
                        System.out.println("Time out !!!!");
                    }
                }
                if(receivedchunks==totalChunks){
                    System.out.println("Uploaded successfully");
                }
                out.write(buffer,0,bytes);
                out.flush();
            }
            fileInputStream.close();
        }catch (Exception e){
            System.out.println("Error in file sending");
            e.printStackTrace();
        }

    }
}

