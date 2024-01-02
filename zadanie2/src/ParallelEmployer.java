import java.util.*;


public class ParallelEmployer implements Employer {
    private final List<Location> visited = new ArrayList<>();
    private final HashMap<Integer, Object> orders = new HashMap<>();
    private final HashMap<Integer, Result> results = new HashMap<>();
    private OrderInterface order;
    private Location exit = new Location(-1, -1);
    private volatile boolean exitFound = false;

    @Override
    public void setOrderInterface(OrderInterface order) {
        this.order = order;
        this.order.setResultListener(result -> {
            int orderID = result.orderID();
            synchronized (orders.get(orderID)) {
                results.put(orderID, result);
                orders.get(orderID).notify();
            }
        });
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        visited.add(startLocation);
        findExitRecurrent(startLocation, allowedDirections);
        return exit;
    }

    private void findExitRecurrent(Location startLocation, List<Direction> allowedDirections) {
        List<Thread> threads = new ArrayList<>();

        for (var allowedDirection : allowedDirections) {
            if (exitFound) { return; }

            Thread t = new Thread(() -> {
                Location nextStep = allowedDirection.step(startLocation);
                int orderID = 0;
                synchronized (visited) {
                    orderID = orderFindingExit(nextStep, orderID);
                }
                if (orders.containsKey(orderID)) {
                    synchronized (orders.get(orderID)) {
                        Result res = getResult(orderID);
                        if (res.type() == LocationType.EXIT) {
                            exit = nextStep;
                            exitFound = true;
                        } else {
                            findExitRecurrent(nextStep, res.allowedDirections());
                        }
                    }
                }
            });

            t.start();
            threads.add(t);
        }

        joinThreads(threads);
    }

    private int orderFindingExit(Location nextStep, int orderID) {
        if (!visited.contains(nextStep)) {
            visited.add(nextStep);
            orderID = order.order(nextStep);
            orders.put(orderID, new Object());
        }
        return orderID;
    }

    private Result getResult(int orderID) {
        try {
            orders.get(orderID).wait();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        return results.get(orderID);
    }

    private static void joinThreads(List<Thread> threads) {
        for (var t : threads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }
}
