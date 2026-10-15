# ECLYRA — Vertical slice nel browser (prova su telefono)

**Scopo**: giocare la vertical slice dal telefono (Android o iPhone) senza installare niente, per i test delle fasi B, D ed E.

È una build di prova della slice. Non sostituisce:
- l'APK Android;
- il test iOS reale (fase A).

Il gioco principale non cambia.

## 1. Come funziona

- La slice si compila in WebAssembly con emscripten. Sono gli stessi file C della versione desktop; cambia solo il ciclo principale, come nel browser richiede `emscripten_set_main_loop`.
- La GitHub Action `.github/workflows/slice_web.yml`:
  - compila la slice a ogni push su `main`;
  - pubblica il sito su **GitHub Pages**, gratis.
- Indirizzo: `https://<tuo-utente>.github.io/<nome-repository>/`.
- Il sito pesa circa 1 MB: pagina, codice, asset.

| Cosa | Comportamento |
|---|---|
| Telefono / tablet | Controlli touch automatici (croce, A, B, MENU; carte delle mosse toccabili). |
| Schermo intero | GIOCA apre lo schermo intero. Su Android blocca anche l'orizzontale. |
| In verticale | Avviso «Ruota il telefono». |
| PC | Tastiera e controller, come la versione desktop. |
| «Esci» nel titolo | Torna alla schermata GIOCA. |
| Forzare i controlli | `?touch=1` li forza, `?touch=0` li toglie. |

**Come un'app**: «Aggiungi a schermata Home» dal menu del browser crea un'icona.
- Su Android si apre a schermo intero e in orizzontale.
- Su iPhone si apre senza la barra di Safari. iPhone non permette lo schermo intero vero da Safari: l'icona sulla Home è il modo migliore.

## 2. Pubblicarla (una volta sola, dal Mac)

1. Su github.com crea un repository **vuoto**:
   - nome: per esempio `eclyra`;
   - senza README e senza licenza;
   - pubblico, oppure privato con un piano che includa Pages.
2. Nel Terminale, dalla cartella del progetto:
   ```bash
   cd ~/percorso/ECLYRA
   git init                      # solo se la cartella non è già un repository
   git add .
   git commit -m "ECLYRA 0.4.3: slice web"
   git branch -M main
   git remote add origin https://github.com/<tuo-utente>/eclyra.git
   git push -u origin main
   ```
   Al primo push GitHub chiede di accedere. Come password serve un *token*, non quella del sito:
   - si crea in Settings → Developer settings → Personal access tokens;
   - oppure si installa GitHub Desktop, che lo gestisce da solo.
3. Su GitHub apri **Settings → Pages**. In *Build and deployment → Source* scegli **GitHub Actions**.
4. Apri la scheda **Actions** e controlla il workflow `slice-web`:
   - se è fallito perché Pages non era ancora attivo, premi **Re-run all jobs**;
   - in 2–3 minuti compare il link verde al sito.

Da lì in poi ogni `git push` aggiorna il sito da solo.

## 3. Compilarla a mano (facoltativo)

Serve emsdk (https://emscripten.org), versione 3.1.74 come nella CI.

```bash
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
cd build-web/slice && python3 -m http.server 8000      # poi apri http://localhost:8000
```

Il file va aperto da un server: aprirlo con doppio clic non funziona.

## 4. Verifiche fatte

Prove in Chromium:
- **PC** 1280×720:
  - titolo;
  - scelta dello starter con tastiera;
  - arrivo a Lunaria.
- **Telefono** 844×390, touch:
  - GIOCA;
  - titolo e starter con il dito;
  - camminata con la croce tenuta;
  - MENU;
  - avviso in verticale.

Il canvas resta a scala intera (400×195 ×2 sul telefono di prova).

I test sui telefoni veri li fai tu, con i registri:
- `docs/test/TEST_PRIMA_LOTTA.md`;
- la sezione D di `docs/VERTICAL_SLICE.md`.

## 5. Limiti

- Come la slice desktop: niente audio, salvataggi e interni.
- Le prestazioni nel browser non sono quelle dell'app finale. Per quelle serviranno l'APK e il test iOS reale.
