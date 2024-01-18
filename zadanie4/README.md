<h3>Zadanie</h3>

<h4>Idea</h4>

<p>
Program prowadzi symulację za pomocą dynamiki molekularnej pewnej liczby czątek oddziałujących
na siebie pewną siła.
</p>

<h4>Wersja sekwencyjna kodu</h4>

<p>Kod sekwencyjny jest załączony. Trzeba poddać go modyfikacjom za pomocą OpenMP.</p>

<h4>Zadanie</h4>

<p>Zadanie polega na użyciu OpenMP w celu przyspieszenia obliczeń.</p>

<p>Modyfikacjom ma zostać poddany wyłącznie kod znajdujący się w klasie Simulation.</p>
<p>Generalnie, zadanie należy uznać za proste...</p>

<p>Wymagania dla aplikacji równoległej</p>

<ul>
<li>Wersja równoległa i sekwencyjna muszą dawać ten sam wynik.
</li><li>Przyspieszyć należy <b>wszystkie</b> nadające się do zrównoleglenia metody
klasy Simulation. Koniecznie trzeba zadbać o szybsze działanie samej symulacji 
i krótszy czas realizacji metod pairDistribution i avgMinDistance.
</li><li>Proszę nie zmieniać algorytmu/optymalizować pracy programu. Czas wykonania sekwencyjnej wersji 
oryginalnego kodu i po Państwa poprawkach ma być zbliżony. Poprawa szybkości pracy ma być
związana ze zrównolegleniem kodu.
</li><li>Dodatkowe wątki mają przyspieszać obliczenia!
</li><li>Proszę nie zmieniać liczby tworzonych wątków. Ma działać ich dokładnie tyle, ile zostanie
ustalone poprzez zmienną środowiskową OMP_NUM_THREADS.
</li><li>Program ma działać efektywnie - tj. dostając do dyspozycji kolejne rdzenie
ma skracać czas potrzebny do uzyskania efektu.
</li><li>Kod musi dać się skompilować i uruchomić na udostępnianych komputerach Wydziałowych (klaster).
</li></ul>

<h4>Dostarczanie rozwiązania:</h4>
<ul>
<li>Rozwiązanie to klasa <code>Simulation</code>
</li><li>Proszę nie zmieniać interfejsu klasy (metod używanych do komunikacji z użytkownikiem).
</li><li>Nagłówek o nazwie <tt>Simulation.h</tt> - proszę dostarczyć, tylko jeśli wprowadzone zostały w nim zmiany.
</li><li>Pliki .cpp i .h proszę wgrywać do systemu osobno.
</li><li>Uwaga: jeśli ktoś z Państwa chce dostarczyć więcej niż jedno rozwiązanie 
w ramach jednego terminu, to proszę zadbać o ponowne wgranie zarówno plików
.cpp, jak i jeśli jest taka potrzeba, odpowiedniego pliku .h - nazwy tych
plików muszą do siebie pasować. 
</li><li>Warto zadbać o to, aby Państwa kod nie wyświetlał komunikatów na terminalu.
Coś takiego może znacząco pogorszyć efektywności pracy programu.
</li><li>Proszę za pomocą include wskazywać wyłącznie pliki nagłówkowe
</li></ul>

<h4>Typowe błędy w rozwiązaniu zadania</h4>

<ul>
<li>Zmienne zadeklarowane przed "#pragma omp parallel" są domyślnie zmiennymi współdzielonymi.
Nie każdej można tak używać. Trzyba użyć private.
</li><li>Doprowadzenie do wystąpienia zagnieżdżenia bloków parallel. To nie jest dobry pomysł. Zazwyczaj doprowadzi do fatalnej
efektywności kodu.
</li><li>Brak ochrony współdzielonych, modyfikowanych zmiennych prowadzi do błędów.
</li><li>Sekcje krytyczne blokują współbieżne operacje
</li><li>Nie wszystkie iteracji zrównoleglanej pętli trwają tyle samo. Użycie schedule i odpowidnich ustawień mile widziane.
</li></ul>

<h4>Tester</h4>

<p>Udostępniony został program testujący. Sposób kompilacji podobny do tego z MPI.:</p>

<pre>java -jar /cluster-app/clientOMP.jar 192.168.1.16 Simulation.h Simulation.cpp
</pre>

<p>Domyślnie wykonywane są wszystkie testy. Uruchomienie wybranego odbywa się poprzez
podanie parametru liczbowego.</p>

<p>Poszczególne bity podanej liczby kodują następujące testy:</p>

<pre>8    serialVsParallel
16   correctnessTest
32   nestedParallel
64   simulationTestParabola
128  simulationTestMyForce
</pre>

<p>Uruchomienie programu testującego tak:</p>

<pre>./a.out 40
</pre>

<p>uaktywni testy: serialVsParallel i nestedParallel.</p>

<p>Uwaga: test nie jest w stanie wykryć błędów wynikających ze złęgo użycia zmiennych
współdzielonych. Rzecz co najwyżej pojawi się jako ograniczenie wydajności. Niestety, w tym przypadku
trzeba przeglądnąć kod źródłowy.</p>

<h5>Testy</h5>

<p>serialVsParallel - wykonuje mój i Państwa kod sekwencyjnie. Oczekiwany jest identyczny wynik i 
podobny czas wykonania</p>
<p>correctnessTest - wyliczane są wyniki symulacji. Wersja sekwencyjna (moja) i równoległa (Państwa) mają dać taki sam wynik.
Dodatkowo przeprowadzany jest test efektywności pracy.</p>
<p>nestedParallel - testu uruchamia zagnieżdżone zrównoleglenia i sprawdza tego efekty</p>
<p>simulationTestParabola, simulationTestMyForce - testy liczą pewną liczbę kroków symulacji i oczekują 
identycznych wyników pracy kodu mojego i Państwa.
