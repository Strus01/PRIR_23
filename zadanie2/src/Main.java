import java.sql.Array;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

class Res implements Result {
    int orderID;
    LocationType type;
    List<Direction> allowedDirections;

    public Res(int orderID, LocationType type, List<Direction> allowedDirections) {
        this.orderID = orderID;
        this.type = type;
        this.allowedDirections = allowedDirections;
    }

    @Override
    public int orderID() {
        return this.orderID;
    }

    @Override
    public LocationType type() {
        return this.type;
    }

    @Override
    public List<Direction> allowedDirections() {
        return this.allowedDirections;
    }
}

class Order implements OrderInterface {
    ResultListener listener;
    private LocationType[][] labirynth;
    static int lastOrderID = 0;

    static HashMap<Location, Integer> locationsOrdered = new HashMap<>();

    @Override
    public void setResultListener(ResultListener listener) {
        this.listener = listener;
    }

    public void setLabirynth(LocationType[][] labirynth) {
        this.labirynth = labirynth;
    }

    private void sendResult(Result result) {
        try {
            long sleepTime = (long)(Math.random() * 500);
            Thread.sleep(sleepTime);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        this.listener.result(result);
    }

    @Override
    public int order(Location location) {
        // increment lastOrderID
        lastOrderID += 1;

        // check neighbours and add them to allowedDirections
        ArrayList<Direction> allowedDirections = new ArrayList<>();
        if(location.col() > 0 && labirynth[location.col()-1][location.row()] != LocationType.WALL) {
            allowedDirections.add(Direction.WEST);
        }
        if(location.col() < 17 && labirynth[location.col()+1][location.row()] != LocationType.WALL) {
            allowedDirections.add(Direction.EAST);
        }
        if(location.row() > 0 && labirynth[location.col()][location.row()-1] != LocationType.WALL) {
            allowedDirections.add(Direction.SOUTH);
        }
        if(location.row() < 15 && labirynth[location.col()][location.row()+1] != LocationType.WALL) {
            allowedDirections.add(Direction.NORTH);
        }
        Result result = new Res(lastOrderID, labirynth[location.col()][location.row()], allowedDirections);

        System.out.println("Created a order with ID " + lastOrderID + " to visit [" + location.col() + ", " + location.row() + "]");

        if(locationsOrdered.containsKey(location)) {
            // add 1 to the number of orders for this location
            locationsOrdered.put(location, locationsOrdered.get(location) + 1);
        } else {
            // add this location to the map
            locationsOrdered.put(location, 1);
        }

        // create a new thread for this result
        Runnable r = () -> sendResult(result);
        new Thread(r).start();
        return lastOrderID;
    }

    public void printLocationsOrdered() {
        System.out.println("Locations ordered:");
        for(Map.Entry<Location, Integer> entry : locationsOrdered.entrySet()) {
            System.out.println("[" + entry.getKey().col() + ", " + entry.getKey().row() + "] -> " + entry.getValue());
        }
    }
}

public class Main {
    public static void main(String[] args) {

        ParallelEmployer employer = new ParallelEmployer();
        Order order = new Order();

        employer.setOrderInterface(order);

        Location startLocation = new Location(9, 7);
        LocationType[][] labirynth = new LocationType[16][18];

        char[][] values = {
                {'W', 'W', 'W', 'E', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'}, // 15
                {'W', 'W', 'W', 'P', 'W', 'W', 'P', 'P', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W'}, // 14
                {'W', 'P', 'W', 'P', 'W', 'P', 'P', 'P', 'W', 'P', 'W', 'W', 'W', 'P', 'W', 'W', 'W', 'W'}, // 13
                {'W', 'P', 'W', 'P', 'W', 'P', 'P', 'W', 'W', 'P', 'W', 'W', 'W', 'P', 'P', 'W', 'W', 'W'}, // 12
                {'W', 'P', 'P', 'P', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'W', 'P', 'P', 'P', 'W', 'W'}, // 11
                {'W', 'P', 'W', 'P', 'W', 'W', 'P', 'P', 'P', 'W', 'P', 'W', 'W', 'W', 'W', 'P', 'W', 'W'}, // 10
                {'W', 'P', 'W', 'P', 'W', 'W', 'P', 'P', 'P', 'W', 'P', 'W', 'W', 'W', 'W', 'P', 'W', 'W'}, // 9
                {'W', 'P', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'P', 'P', 'W', 'P', 'W', 'W'}, // 8
                {'W', 'P', 'W', 'P', 'P', 'W', 'W', 'W', 'P', 'P', 'P', 'W', 'P', 'P', 'W', 'P', 'W', 'W'}, // 7
                {'W', 'P', 'W', 'P', 'P', 'W', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'P', 'W', 'W'}, // 6
                {'W', 'P', 'W', 'P', 'P', 'W', 'P', 'W', 'P', 'P', 'W', 'W', 'W', 'W', 'W', 'P', 'W', 'W'}, // 5
                {'W', 'P', 'W', 'P', 'W', 'W', 'P', 'W', 'P', 'W', 'W', 'W', 'W', 'W', 'W', 'P', 'W', 'W'}, // 4
                {'W', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'P', 'P', 'P', 'P', 'W', 'W', 'W', 'P', 'W', 'W'}, // 3
                {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'P', 'W', 'W', 'W', 'P', 'P', 'W', 'W'}, // 2
                {'W', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W'}, // 1
                {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'}  // 0
        };

        for(int i=0; i<16; i++) {
            for(int j=0; j<18; j++) {
                if(values[i][j] == 'W')
                    labirynth[i][j] = LocationType.WALL;
                else if(values[i][j] == 'P')
                    labirynth[i][j] = LocationType.PASSAGE;
                else if(values[i][j] == 'E')
                    labirynth[i][j] = LocationType.EXIT;
            }
        }

        // print the labirynth
        for(int i=0; i<16; i++) {
            for(int j=0; j<18; j++) {
                if(labirynth[i][j] == LocationType.WALL)
                    System.out.print("W ");
                else if(labirynth[i][j] == LocationType.PASSAGE)
                    System.out.print("P ");
                else if(labirynth[i][j] == LocationType.EXIT)
                    System.out.print("E ");
            }
            System.out.println();
        }
        System.out.println();

        order.setLabirynth(labirynth);

        Location result = employer.findExit(startLocation, new ArrayList<Direction>(List.of(Direction.NORTH)));
        System.out.println("END RESULT: " + result);

        // print hashmap locationsOrdered
        order.printLocationsOrdered();

        try {
            //System.out.println("Sleeping for 10 seconds to wait for all threads to finish");
            Thread.sleep(10000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}