<h4>Idea</h4>

<p>
Program prowadzi symulację podobną do <a href="https://pl.wikipedia.org/wiki/Gra_w_%C5%BCycie">Gry w życie</a>.
Zasady zabawy zostały jednak zmienione i dodany został dodatkowy czynnik w postaci zanieczyszczenia środowiska.
Zanieczyszczenia umożliwiają zmiany reguł: np. duże zanieczyszczenie wpływa na przeżywalność komórek.
</p>

<p>Działanie programu można zobaczyć w postaci prostej animacji.
Po lewej stronie stan komórek. Po stronie prawej poziom zanieczyszczenia.
Uwaga: jedna komórka ma szerokość dwóch znaków. Przepraszam za artefakty, które wygenerowały się 
w trakcie zapisu stanu terminala.</p>

<h4>Wersja sekwencyjna kodu</h4>

<p>Udostępniam kod w wersji sekwencyjnej. Implementacja interfejsu Life
to <code>LifeSequentialImplementation</code>. UWAGA: ta wersja działa <b>tylko</b> z <b>jednym</b> procesem.
<code>Main.cpp</code> uwzględnia MPI - dzięki tej części kodu można zobaczyć jak funkcjonować 
będzie wywoływanie metod <code>Life</code> w wersji współbieżnej. Warto zwrócić uwagę na to,
które z metod (i kiedy) wykonywane są wyłącznie w procesie o numerze 0, a jakie 
wykonane zostaną wszystkimi procesami.</p>

<h4>Zadanie</h4>

<p>Zadanie polega na napisaniu kodu, który używając MPI przyspieszy obliczenia.</p>

<p>Wymagania dla aplikacji równoległej</p>

<ul>
<li>Wersja równoległa i sekwencyjna muszą dawać ten sam wynik.
<li>Dodatkowe procesy mają przyspieszać obliczenia.
<li>Wszystkie procesy mają uczestniczyć w obliczeniach. Czyli, już dwa procesy mają przyspieszyć 
rachunek.
<li>Program ma działać efektywnie - tj. dostając do dyspozycji kolejne rdzenie/procesory
ma skracać czas potrzebny do uzyskania efektu.
<li>Kod musi dać się skompilować i uruchomić na udostępnianych komputerach Wydziałowych (klaster).
Proszę używać MPI w wersji OpenMPI.
</ul>

<h4>Dostarczanie rozwiązania:</h4>
<ul>
<li>Rozwiązanie (klasa <code>LifeParallelImplementation</code>) ma rozszerzać 
klasę <code>Life</code>
<li>Klasa <code>LifeParallelImplementation</code> ma posiadać konstruktor bezparametrowy.
<li>Nagłówek o nazwie <tt>LifeParallelImplementation.h</tt> - proszę również dostarczyć. 
<li>Pliki .cpp i .h proszę wgrywać do systemu osobno.
<li>Uwaga: jeśli ktoś z Państwa chce dostarczyć więcej niż jedno rozwiązanie 
w ramach jednego terminu, to proszę zadbać o ponowne wgranie zarówno plików
.cpp, jak i jeśli jest taka potrzeba, odpowiedniego pliku .h - nazwy tych
plików muszą do siebie pasować. Czyli np. LifeParallelImplementation2.cpp i LifeParallelImplementation2.h
Proszę jednak w include nadal używać LifeParallelImplementation.h (pliki otrzymają 
odpowiednie nazwy przed ich kompilacją).
<li>Warto zadbać o to, aby Państwa kod nie wyświetlał komunikatów na terminalu.
Coś takiego może znacząco pogorszyć efektywności pracy programu.
<li>Proszę za pomocą include wskazywać wyłącznie pliki nagłówkowe (.h) 
</ul>