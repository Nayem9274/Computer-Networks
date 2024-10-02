package Client;

import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Download extends Thread {

    private Socket s;
    private DataInputStream in;
    private DataOutputStream out;
    private String privacy;
    private String src;
    private String dest;
    private String fileName;
    private long fileSize;
    private int chunkSize;

    public Download(Socket s, String fileName,String privacy, String src,String dest, int chunkSize) throws IOException {
        this.s = s;
        this.privacy = privacy;
        this.fileName = fileName;
        this.src = src;
        this.dest = dest;
        this.chunkSize = chunkSize;
        in = new DataInputStream(s.getInputStream());
        //out = new DataOutputStream(s.getOutputStream());

    }

    @Override
    public void run()
    {
        try {
            int bytes = 0;
            FileOutputStream fileOutputStream = new FileOutputStream(fileName);
            long size = in.readLong();     // read file size
            byte[] buffer = new byte[1<<15];
            while (size > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, size))) != -1) {
                fileOutputStream.write(buffer,0,bytes);
                size -= bytes;      // read upto file size
            }
            fileOutputStream.close();
            // The Files.move method is used to move or rename a file, and Paths.get is used to obtain a Path object representing a file or directory path.
            if(src.equalsIgnoreCase("no")|| dest.equalsIgnoreCase("no")){
                File file = new File(src+"/Downloads");
                file.mkdir();
                Files.move(Paths.get(fileName), Paths.get(src+"/Downloads/"+fileName));
            }else{
                Files.move(Paths.get(src), Paths.get(dest));
            }
            System.out.println("Download Completed");

        }catch(Exception e)
        {
            //System.out.println("Error in Downloading!!");
            //e.printStackTrace();
        }
    }
}
