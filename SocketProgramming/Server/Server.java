package Server;
import java.io.DataOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.util.*;

class Client {
    private String username;
    private StringBuilder messageBuilder;

    public Client(String username) {
        this.username = username;
        messageBuilder = new StringBuilder();
    }

    public String getUserName() {
        return username;
    }

    public void addMessage(String message) {
        messageBuilder.append(message).append("\n");
    }

    public String checkMessages() {
        return messageBuilder.toString();
    }

    public void clearMessages() {
        messageBuilder.setLength(0);
    }
}

class Request {
    Client c;
    String filename;

    public Request(Client c, String filename) {
        this.c = c;
        this.filename = filename;
    }

    public  Client getUser() {
        return c;
    }

    public String getFilename() {
        return filename;
    }
}


public class Server {

    public static List<Socket> clientSockets1;
    public static List<Socket> clientSockets2;
    public static HashMap<Socket, Client> socketUserHashMap1;
    public static HashMap<Socket, Client> socketUserHashMap2;
    public static List<Request> requests;
    public static List<String> singleUser;
    public static Set<String> sUser;
    public static int userCount = 0;
    static int MAX_CHUNK_SIZE = 1<<8;
    static int MIN_CHUNK_SIZE = 1<<5;

    static int MAX_BUFFER_SIZE = 999999999;
    static int AVAILABLE_BUFFER = MAX_BUFFER_SIZE;


    public static int genChunkSize(){
        Random random = new Random();
        int range = MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1;
        int chunkSize = random.nextInt(range) + MIN_CHUNK_SIZE;
        return chunkSize;
    }

    public static void main(String[] args) throws Exception{
        // 2 sockets : 1 message socket , 1 file socket (download/upload)
        ServerSocket serverSocket1 = new ServerSocket(8866);
        ServerSocket serverSocket2 = new ServerSocket(8877);

        clientSockets1 = new ArrayList<>();
        clientSockets2 = new ArrayList<>();
        socketUserHashMap1 = new HashMap<Socket, Client>();
        socketUserHashMap2 = new HashMap<Socket, Client>();
        singleUser = new ArrayList<>(); // list of users to the server
        sUser = new HashSet<>();


        requests = new ArrayList<Request>(); // list of requests

        // continuously reads input from the console and broadcasts it to all connected clients.
        Thread serveroperator = new Thread(() -> {
            try {
                while (true) {
                    Scanner scanner = new Scanner(System.in);
                    String message = scanner.nextLine();
                    for (int i = 0; i < clientSockets1.size(); i++) {
                        if (!clientSockets1.get(i).isClosed()) {
                            DataOutputStream dataOutputStream = new DataOutputStream(clientSockets1.get(i).getOutputStream());
                            dataOutputStream.writeUTF(message);
                            dataOutputStream.flush();
                        }
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        serveroperator.start();

        while(true){
            Socket socket1 = serverSocket1.accept();
            clientSockets1.add(socket1);

            Socket socket2 = serverSocket2.accept();
            clientSockets2.add(socket2);

            userCount++;

            SystemServer s= new SystemServer(socket1, socket2);
            s.start();
        }
    }


}

