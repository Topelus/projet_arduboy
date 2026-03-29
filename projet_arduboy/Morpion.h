// ==== Écran de fin (Victoire ou Défaite) ====
void showGameOver() {
  int lastScore = currentGame->getScore();
  int high = getHighScore(currentGameId);
  if (lastScore > high) {
    setHighScore(currentGameId, lastScore);
    high = lastScore;
  }

  bool isVictory = currentGame->isVictory();  // Vérifie si c'est une victoire

  screen.fillScreen(TFT_BLACK);
  
  if (isVictory) {
    // ═══════════════════════════════════════════════════
    // ÉCRAN DE VICTOIRE (Vert)
    // ═══════════════════════════════════════════════════
    screen.fillRect(0, 0, 240, 40, TFT_DARKGREEN);
    screen.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    screen.setTextSize(3);
    screen.setCursor(35, 10);
    screen.print("VICTOIRE!");
    
    // Son de victoire
    tone(BUZZER_PIN, 523, 150); delay(150);  // Do
    tone(BUZZER_PIN, 659, 150); delay(150);  // Mi
    tone(BUZZER_PIN, 784, 150); delay(150);  // Sol
    tone(BUZZER_PIN, 1047, 300); delay(300); // Do (octave haute)
    noTone(BUZZER_PIN);
    
  } else {
    // ═══════════════════════════════════════════════════
    // ÉCRAN DE DÉFAITE (Rouge)
    // ═══════════════════════════════════════════════════
    screen.fillRect(0, 0, 240, 40, TFT_MAROON);
    screen.setTextColor(TFT_WHITE, TFT_MAROON);
    screen.setTextSize(3);
    screen.setCursor(30, 10);
    screen.print("GAME OVER");
    
    // Son de défaite (grave)
    tone(BUZZER_PIN, 200, 300); delay(300);
    tone(BUZZER_PIN, 150, 500); delay(500);
    noTone(BUZZER_PIN);
  }
  
  // Affichage du score (commun aux deux cas)
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setTextSize(2);
  screen.setCursor(20, 55);
  screen.print("Score: ");
  screen.print(lastScore);
  
  screen.setTextColor(TFT_YELLOW, TFT_BLACK);
  screen.setCursor(20, 80);
  screen.print("Record: ");
  screen.print(high);
  
  // Instructions
  screen.setTextColor(TFT_CYAN, TFT_BLACK);
  screen.setTextSize(1);
  screen.setCursor(20, 110);
  screen.print("[A] Rejouer    [B] Menu");
}