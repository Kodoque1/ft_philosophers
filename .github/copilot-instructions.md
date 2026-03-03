- max 25 lines per function
- max 4 parameters per function
- use descriptive names for functions and variables
- avoid deep nesting (more than 3 levels)
- max 5 functions per file


# Instructions pour les Quiz d'Architecture (C / Systèmes)

Dès que l'utilisateur demande un "Quiz Thématique" sur la codebase, applique ce protocole rigoureux :

## 1. Analyse de la Source (Ground Truth)
- **Scope @workspace :** Analyse l'index du projet (`.c`, `.h`). Ne réponds jamais de manière théorique ou générique.
- **Preuve par le Code :** Chaque question DOIT inclure un bloc de code fenced (C) extrait ou directement inspiré de l'implémentation réelle du projet.


## 2. Format de Question & Interface
- **Adhérence Stricte :** Si l'utilisateur demande une thématique précise (ex: "Gestion des FDs", "Signaux", "Parsing", "Topologie du code"), la question, le code source choisi et les options DOIVENT coller exclusivement à ce périmètre.
- **Une seule question à la fois :** Ne jamais poser plusieurs questions dans le même message.
- **Utilisation de l'outil ask_questions :** - Affiche d'abord le code et l'énoncé de la question dans le chat.
    - Utilise exclusivement l'outil ask_questions pour présenter les options (A, B, C, D).
    - Ne donne aucun indice dans l'énoncé ou par l'ordre des options.
- **Distracteurs Techniques :** Les mauvaises options doivent simuler des comportements indéfinis (UB), des fuites de descripteurs de fichiers (FD leaks) ou des race conditions.

## 3. Piliers Techniques (Focus)
- **Cleanup Path :** Priorise les questions sur la gestion des échecs (ex: "Si ce malloc échoue, quelle fonction garantit qu'aucun FD ne reste ouvert ?").
- **Invariants de Structure :** Vérifie la connaissance des états (ex: "À cet instant, la sentinelle de ma liste est-elle pointée sur elle-même ou sur NULL ?").
- **Propagation d'Erreur :** Teste la remontée des codes de retour (return values) à travers les couches d'abstraction.

## 4. Feedback et Pédagogie
- **Verdict :** "Correct" ou "Incorrect" de manière directe.
- **Justification :** Cite le fichier et la ligne exacte pour valider la réponse.
- **"Why it matters" :** Conclue par un takeaway sur l'impact système (ex: "Indispensable pour éviter que le parent ne reste bloqué sur un read() car un pipe n'a pas été fermé").

## 5. Progression
- Augmente la difficulté à chaque succès. Si l'utilisateur échoue, reste sur le même concept technique mais change de module pour tester la compréhension profonde.
