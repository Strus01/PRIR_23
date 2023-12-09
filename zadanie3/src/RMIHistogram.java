import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.*;

public class RMIHistogram extends UnicastRemoteObject implements RemoteHistogram, Binder  {

    private int id;
    private Map<Integer, ArrayList<Integer>> histograms;

    protected RMIHistogram() throws RemoteException {
        super();
        histograms = new HashMap<>();
    }

    @Override
    public void bind(String serviceName) {
        try {
            Registry registry = LocateRegistry.getRegistry("localhost", 1099);
            registry.rebind(serviceName, this);

        } catch (RemoteException e) {
            System.err.println(e.getMessage());
        }
    }

    @Override
    synchronized public int createHistogram(int bins) throws RemoteException {
        id++;
        ArrayList<Integer> histogram = new ArrayList<>(Collections.nCopies(bins, 0));
        histograms.put(id, histogram);
        return id;
    }

    @Override
    synchronized public void addToHistogram(int histogramID, int value) throws RemoteException {
        ArrayList<Integer> histogram = histograms.get(histogramID);
        int currentValue = histogram.get(value);
        histogram.set(value, currentValue + 1);
    }

    @Override
    synchronized public int[] getHistogram(int histogramID) throws RemoteException {
        ArrayList<Integer> histogram =  histograms.get(histogramID);
        return histogram.stream().mapToInt(Integer::intValue).toArray();
    }
}
