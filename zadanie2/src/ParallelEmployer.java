import java.util.*;


public class ParallelEmployer implements Employer {
    private final List<Location> visited = new ArrayList<>();
    private final HashMap<Integer, Object> threads = new HashMap<>();
    private final HashMap<Integer, Result> threadsResults = new HashMap<>();
    private OrderInterface order;
    private Location exit = new Location(-1, -1);
    private final Object lock = new Object();
    private volatile boolean exitFound = false;

    @Override
    public void setOrderInterface(OrderInterface order) {
        this.order = order;
        this.order.setResultListener(new ResultListener() {
            @Override
            public void result(Result result) {
                int orderID = result.orderID();
                synchronized (threads.get(orderID)) {
                    threadsResults.put(orderID, result);
                    threads.get(orderID).notify();
                }
            }
        });
    }

    @Override
    public Location findExit(Location startLocation, List<Direction> allowedDirections) {
        visited.add(startLocation);
        findExitRecurrent(startLocation, allowedDirections);
//        synchronized (lock) {
//            try {
//                lock.wait();
//            } catch (InterruptedException e) {
//                throw new RuntimeException(e);
//            }
            return exit;
//        }
    }

    private void findExitRecurrent(Location startLocation, List<Direction> allowedDirections) {
        for (Direction allowedDirection : allowedDirections) {
            if (exitFound) {
                return;
            }
            Thread t = new Thread(() -> {
                Location nextStep = allowedDirection.step(startLocation);
                int orderID = 0;
                synchronized (visited) {
                    orderID = orderFindingExit(nextStep, orderID);
                }
                if (threads.containsKey(orderID)) {
                    synchronized (threads.get(orderID)) {
                        try {
                            threads.get(orderID).wait();
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                        Result res = threadsResults.get(orderID);;
                        if (res.type() == LocationType.EXIT) {
                            exit = nextStep;
                            exitFound = true;
//                            synchronized (lock) {
//                                lock.notify();
//                            }

                        } else {
                            findExitRecurrent(nextStep, res.allowedDirections());
                        }
                    }
                }
            });
            t.start();
            
            try {
                t.join();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private int orderFindingExit(Location nextStep, int orderID) {
        if (!visited.contains(nextStep)) {
            visited.add(nextStep);
            orderID = order.order(nextStep);
            threads.put(orderID, new Object());
        }
        return orderID;
    }
}
