package Client;

import Server.Server;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;



public class Client extends Thread {

    public static List<String> inbox;
    public static String username;
    public static void main(String[] args) throws Exception{
        Socket socket1 = new Socket("localhost",8866);
        Socket socket2 = new Socket("localhost", 8877);
        System.out.println("Remote port: " + socket1.getPort());
        System.out.println("Remote port: " + socket2.getPort());
        inbox = new ArrayList<>();

        // handle messages received from a client.
        Thread reader = new Thread (() -> {
            try {
                DataInputStream dataInputStream=new DataInputStream(socket1.getInputStream());;
                DataOutputStream dataOutputStream=new DataOutputStream(socket1.getOutputStream());
                while (true) {
                    if (socket1.isClosed() || socket2.isClosed()) {
                        break;
                    }

                    String message = dataInputStream.readUTF();
                    String[] arr = message.split("\\ ", 2);

                    if (message.equalsIgnoreCase("logout")) {
                        System.out.println("You are logged out");
                        break;
                    }

                    System.out.println(message);

                    if (arr[0].equalsIgnoreCase("download")) {
                        try {
                            int chunkSize = dataInputStream.readInt();
                            String[] filename = arr[1].split("_", 3);
                            String src = "Data/"+username;
                            Download x= new Download(socket2,filename[2],"no",src,"no",chunkSize);
                            x.start();

                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                    if (arr[0].equalsIgnoreCase("upload")) {
                        System.out.println("Uploading...");

                        String filePath = dataInputStream.readUTF();
                        System.out.println(filePath);
                        long fileSize = dataInputStream.readLong();
                        int chunkSize = dataInputStream.readInt();

                        dataOutputStream.writeUTF("upload " + arr[1] + " " + filePath);
                        dataOutputStream.flush();
                        dataOutputStream.writeLong(fileSize);
                        dataOutputStream.flush();
                        dataOutputStream.writeInt(chunkSize);
                        dataOutputStream.flush();

                        Upload x = new Upload(socket2, filePath, arr[1], fileSize, chunkSize);
                        x.start();
                    }


                }

                dataInputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        reader.start();



        Thread writer = new Thread(() -> {
            try {
                while (true) {
                    DataOutputStream dataOutputStream = new DataOutputStream(socket1.getOutputStream());
                    System.out.println("1. Login");
                    System.out.println("2. Lookup Clients");
                    System.out.println("3. Make Request");
                    System.out.println("4. Upload private");
                    System.out.println("5. Upload public");
                    System.out.println("6. Download");
                    System.out.println("7. Inbox");
                    System.out.println("8. View files");
                    System.out.println("9. Logout");

                    Scanner scanner = new Scanner(System.in);
                    String message = scanner.nextLine();

                    switch (message) {
                        case "1":
                            System.out.println("Your Username:");
                            username = scanner.nextLine();
                            message = "login " + username;
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "2":
                            message = "online";
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "3":
                            System.out.println("Requested File Name: ");
                            String fileName = scanner.nextLine();
                            message = "request " + fileName;
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "4":
                            System.out.println("File name: ");
                            String privateFileName = scanner.nextLine();
                            message = "uploadf private " + privateFileName;
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            File privateFile = new File(privateFileName);
                            dataOutputStream.writeLong(privateFile.length());
                            dataOutputStream.flush();
                            break;
                        case "5":
                            System.out.println("File name: ");
                            String publicFileName = scanner.nextLine();
                            message = "uploadf public " + publicFileName;
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            File publicFile = new File(publicFileName);
                            dataOutputStream.writeLong(publicFile.length());
                            dataOutputStream.flush();
                            break;
                        case "6":
                            System.out.println("Name_Type_filename: ");
                            String fileId = scanner.nextLine();
                            message = "download " + fileId;
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "7":
                            message = "inbox";
                            System.out.println();
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "8":
                            message = "viewfiles";
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        case "9":
                            message = "logout";
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                            break;
                        default:
                            System.out.println("Invalid input. Please try again.");
                            continue;
                    }


                }

            } catch (Exception e) {
                ;
            }
        });
        writer.start();
    }






}
