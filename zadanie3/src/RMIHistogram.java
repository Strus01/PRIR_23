import java.rmi.RemoteException;

public class RMIHistogram implements RemoteHistogram, Binder  {
    @Override
    public void bind(String serviceName) {

    }

    @Override
    public int createHistogram(int bins) throws RemoteException {
        return 0;
    }

    @Override
    public void addToHistogram(int histogramID, int value) throws RemoteException {

    }

    @Override
    public int[] getHistogram(int histogramID) throws RemoteException {
        return new int[0];
    }
}
