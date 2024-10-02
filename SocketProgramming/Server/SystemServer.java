package Server;

import java.io.*;
import java.net.Socket;

public class SystemServer extends Thread{
    private Socket socket1;
    private Socket socket2;
    private DataInputStream messagein;
    private DataOutputStream messageout;
    StringBuilder requestmessage=new StringBuilder();

    public SystemServer(Socket socket1, Socket socket2) throws IOException {
        this.socket1=socket1;
        this.socket2=socket2;
        messagein= new DataInputStream(socket1.getInputStream());
        messageout= new DataOutputStream(socket1.getOutputStream());
    }

    public void viewFiles(Client user) throws Exception {
        File storageDir = new File("Data");
        File[] userDirs = storageDir.listFiles(File::isDirectory); // retrieve the subdirectories within the "Data" directory.

        StringBuilder message = new StringBuilder();
        // iterating over these directories, checking for a match with the user's username and retrieving the corresponding files from the "private" and "public" subdirectories.
        for (File userDir : userDirs) {
            if (userDir.getName().equalsIgnoreCase(user.getUserName())) {
                File privateDir = new File(userDir, "private");
                addFilesToList(message, userDir.getName(), "private", privateDir);

                // Skip processing public files if private directory not found
                if (!privateDir.exists()) {
                    continue;
                }
            }

            File publicDir = new File(userDir, "public");
            addFilesToList(message, userDir.getName(), "public", publicDir);
            File downloadDir = new File(userDir, "Downloads");

            if (downloadDir.exists()) {
                addFilesToList(message, userDir.getName(), "download", downloadDir);
            }


        }

        messageout.writeUTF(message.toString());
        messageout.flush();
    }

    private void addFilesToList(StringBuilder message, String userName, String fileType, File directory) {
        File[] files = directory.listFiles();
        if (files != null) {
            for (File file : files) {
                message.append(userName)
                        .append("_")
                        .append(fileType)
                        .append("_")
                        .append(file.getName())
                        .append("\n");
            }
        }
    }
    public void Download(Socket s,String path, String privacy, int chunkSize){
        try {
            DataOutputStream dataOutputStream = new DataOutputStream(s.getOutputStream());
            File file = new File(path);
            FileInputStream fileInputStream = new FileInputStream(file);
            System.out.println(path + " Download Procedure Started");

            // sends the file size to the receiving end.
            long fileSize = file.length();
            dataOutputStream.writeLong(fileSize);
            dataOutputStream.flush();

            //  creating a buffer of the specified chunk size to read and send the file content in chunks.
            byte[] buffer = new byte[chunkSize];
            int bytesRead=0;
            //  reading a chunk of bytes from the file into the buffer until the end of the file is reached.
            while ((bytesRead = fileInputStream.read(buffer)) != -1) {
                dataOutputStream.write(buffer, 0, bytesRead);
                dataOutputStream.flush();
            }

            fileInputStream.close();
            System.out.println(path + " Download Completed");
        } catch (Exception e) {
            System.out.println("Error!!");
            e.printStackTrace();
        }


    }


    @Override
    public void run() {

        try{
            String m;
            while (true){
                if(socket1.isClosed()) {break;}
                m = messagein.readUTF();
                System.out.println(m);
                String []arr = m.split("\\ ");
                if(arr[0].equalsIgnoreCase("login")){
                    if(!Server.singleUser.contains(arr[1])){
                        // new user
                        Client user = new Client(arr[1]);
                        // adding in both hashmap message and file socket
                        Server.socketUserHashMap1.put(socket1, user);
                        Server.socketUserHashMap2.put(socket2, user);
                        System.out.println("username : " + arr[1]);
                        // Creating folder directory for new user
                        (new File("Data")).mkdir();
                        (new File("Data/"+arr[1])).mkdir();
                        (new File("Data/"+arr[1]+"/public")).mkdir();
                        (new File("Data/"+arr[1]+"/private")).mkdir();
                        Server.singleUser.add(arr[1]);
                        Server.sUser.add(arr[1]);
                    }
                    else{
                        // old user
                        messageout.writeUTF("User already exists..");
                        messageout.flush();
                        messageout.close();
                        socket1.close();
                        socket2.close();
                    }
                }
                if(!arr[0].equalsIgnoreCase("login")){
                    // This check is performed to verify if the client has successfully logged in before sending this non-login message.
                    if(!Server.socketUserHashMap1.containsKey(socket1)){
                        messageout.writeUTF("You are not logged in.");
                        messageout.flush();
                    }
                    else{
                        if(arr[0].equalsIgnoreCase("upload")){
                            String []split = m.split(" ", 3);
                            long fileSize = messagein.readLong();
                            int chunkSize = messagein.readInt();
                            if(split[1].equalsIgnoreCase("private")){
                                String userName = Server.socketUserHashMap1.get(socket1).getUserName();
                                String tempFileName = split[2];

                                Upload x = new Upload(socket2, userName, tempFileName, "private", tempFileName, "Data/"+userName+"/private/"+tempFileName, fileSize, chunkSize);
                                x.start();
                            }
                            else{
                                String userName = Server.socketUserHashMap1.get(socket1).getUserName();
                                String tempFileName = split[2];
                                Upload y = new Upload(socket2,userName, tempFileName, "public", tempFileName, "Data/"+userName+"/public/"+tempFileName, fileSize, chunkSize);
                                y.start();
                            }
                        }

                        if(arr[0].equalsIgnoreCase("uploadf")){
                            int chunkSize = Server.genChunkSize();
                            long fileSize = messagein.readLong();
                            if(Server.AVAILABLE_BUFFER>fileSize){
                                Server.AVAILABLE_BUFFER -= fileSize;
                                String []split = m.split(" ", 3);
                                messageout.writeUTF("upload "+arr[1]);
                                messageout.flush();
                                String userName = Server.socketUserHashMap1.get(socket1).getUserName();
                                String tempFileName = userName+"_"+split[2];
                                //String x= "C:\\Users\\Asus\\Desktop\\ABDULLAH WASI\\3-2\\CSE 321 322\\Lab\\Assignment_1";
                                messageout.writeUTF(split[2]);//sends the file name to the client.
                                messageout.flush();
                                messageout.writeLong(fileSize);//sends the file size to the client.
                                messageout.flush();
                                messageout.writeInt(chunkSize);//sends the chunk size to the client.
                                messageout.flush();
                            }
                            else{
                                messageout.writeUTF("Buffer is busy.");
                                messageout.flush();
                            }
                        }

                        if(arr[0].equalsIgnoreCase("viewfiles")){
                            viewFiles(Server.socketUserHashMap1.get(socket1)); // Server.socketUserHashMap1.get(socket1) retrieves the User object linked with  socket1 from the socketUserHashMap1 map. This User object represents the user for whom the file list is being generated.
                        }

                        if(arr[0].equalsIgnoreCase("download")){

                            String finalM = m;
                            Thread temp = new Thread(()->
                            {
                                try
                                {
                                    messageout.writeUTF(finalM);
                                    messageout.flush();
                                    messageout.writeInt(Server.MAX_CHUNK_SIZE);
                                    messageout.flush();
                                    String []split = finalM.split("\\ ", 2);
                                    String []filename = split[1].split("_", 3);
                                    String username = Server.socketUserHashMap1.get(socket1).getUserName();
                                    String filePath = "Data/" + filename[0] + "/" + filename[1] + "/" + filename[2];
                                    if(filename[1].equalsIgnoreCase("private")){
                                        if(filename[0].equalsIgnoreCase(username)){
                                            Download(socket2,filePath,"private", Server.MAX_CHUNK_SIZE);

                                        }else{
                                            messageout.writeUTF("Access Denied to others' private files");
                                            messageout.flush();
                                        }
                                    }
                                    else{
                                        Download(socket2,filePath,"public", Server.MAX_CHUNK_SIZE);
                                    }
                                }
                                catch (IOException e) { }
                            });
                            temp.start();
                        }

                        if(arr[0].equalsIgnoreCase("online")){
                            StringBuilder onlineUsers = new StringBuilder();
                            StringBuilder offlineUsers= new StringBuilder();
                            for (Socket socket : Server.clientSockets1) {
                                if (!socket.isClosed() && Server.socketUserHashMap1.containsKey(socket)) {
                                    Client user = Server.socketUserHashMap1.get(socket);
                                    onlineUsers.append(user.getUserName()).append("\n");
                                }
                                //Client user = Server.socketUserHashMap1.get(socket);
                                //offlineUsers.append(user.getUserName()).append("\n");
                            }

                            messageout.writeUTF("All Users :");
                            for (String user : Server.sUser) {
                                //System.out.println(Server.singleUser.size());
                                messageout.writeUTF(user);
                                messageout.flush();
                            }
                           // messageout.flush();
                           // messageout.writeUTF(offlineUsers.toString());
                           // messageout.flush();
                            messageout.writeUTF("Online :");
                            messageout.flush();
                            messageout.writeUTF(onlineUsers.toString());
                            messageout.flush();
                        }

                        if(arr[0].equalsIgnoreCase("request")){
                            String userName = Server.socketUserHashMap1.get(socket1).getUserName(); // retreiving the username associated with current socket
                            String[] split = m.split(" ", 2);// splits the received message into two parts: the command ("request") and the requested file name.
                            String requestMessage = userName + " requested for the file " + split[1];
                            requestmessage.append(userName).append("requested for the file" + split[1]);
                            // sending a request message to all connected clients.
                            for (Socket clientSocket : Server.clientSockets1) {
                                if (!clientSocket.isClosed()) {
                                    Server.socketUserHashMap1.get(clientSocket).addMessage(userName + " requested for file " + split[1]);
                                    DataOutputStream dataOutputStream = new DataOutputStream(clientSocket.getOutputStream());
                                    dataOutputStream.writeUTF(requestMessage);
                                    dataOutputStream.flush();
                                }
                            }
                            // creating a Request object using the username and requested file name, and adding it to the Server.requests list.
                            if (userName.equalsIgnoreCase(Server.socketUserHashMap1.get(socket1).getUserName())) {
                                Request request = new Request(Server.socketUserHashMap1.get(socket1), split[1]);
                                Server.requests.clear();
                                Server.requests.add(request);
                            }
                        }

                        if(arr[0].equalsIgnoreCase("inbox")){
                            Client user = Server.socketUserHashMap1.get(socket1);

                            System.out.println(user.checkMessages());
                            for(int i=0;i<Server.requests.size();i++)
                            {
                                Request r = Server.requests.get(i);
                                messageout.writeUTF(r.c.getUserName()+" requested for file "+ r.getFilename());
                            }
                            messageout.flush();
                            user.clearMessages();

                        }

                        if(arr[0].equalsIgnoreCase("logout")){
                            Client user = Server.socketUserHashMap1.get(socket1);
                            Server.singleUser.remove(user.getUserName());

                            messageout.writeUTF("logout");
                            messageout.flush();
                            messageout.close();
                            socket1.close();
                            socket2.close();
                        }
                    }
                }



            }

        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
