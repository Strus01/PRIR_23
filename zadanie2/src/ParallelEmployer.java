import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ParallelEmployer implements Employer {
    private final List<Location> visited = new ArrayList<>();
    private final Map<Integer, Location> orders = new HashMap<>();
    private OrderInterface orderInterface;
    private Location exit = null;

    @Override
    public void setOrderInterface(OrderInterface order) {
        orderInterface = order;
        orderInterface.setResultListener(new ResultListener() {
            @Override
            public synchronized void result(Result result) {
                Location previous = orders.get(result.orderID());
                if (result.type() == LocationType.EXIT) {
                    exit = previous;
                    synchronized (orders) {
                        orders.notify();
                    }
                }
                if (exit == null) {
                    orderAllLocations(previous, result.allowedDirections());
                }
            }
        });
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        visited.add(startLocation);
        orderAllLocations(startLocation, allowedDirections);
        synchronized (orders) {
            try {
                orders.wait();
            } catch (InterruptedException ignored) {
            }
        }
        return exit;
    }

    private void orderAllLocations(Location startLocation, List<Direction> allowedDirections) {
        for (Direction direction : allowedDirections) {
            Location location = direction.step(startLocation);
            if (!visited.contains(location)) {
                visited.add(location);
                int orderId = orderInterface.order(location);
                orders.put(orderId, location);
            }
        }
    }
}