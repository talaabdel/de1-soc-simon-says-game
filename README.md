# 🎮 Simon Says on FPGA (DE1-SoC)

For our **ECE243: Computer Organization** final project (Mar 2025 – May 2025), my partner **Tara Sangra** and I designed and implemented a fully functional, interactive **Simon Says memory game** on the **DE1-SoC FPGA** using C.  

This project combined **software and hardware design**, allowing us to bring concepts from computer organization, embedded systems, and digital design into a fun and engaging game.  

---

## 🕹️ Game Overview  
- Players must memorize and repeat a random pattern of flashing colored boxes using hardware push keys (`KEY0 – KEY3`).  
- Each box lights up with a **unique audio tone**, making the game both visual and auditory.  
- The sequence grows more complex each round.  
- Players earn **1 point per correct sequence**.  
- A wrong answer resets the score back to **0**.  

---

## ✨ Key Features  
- 🎨 **VGA Graphics** for game visuals, score tracking, and level display  
- ⌨️ **PS/2 Keyboard Integration** — start the game with the spacebar  
- 🔊 **Audio Output** — distinct tones for each button + win/lose sound effects  
- ⚡ **Interrupt-Based Input Handling** for fast and reliable gameplay  
- 📈 **Dynamic Difficulty Scaling** — sequences get faster with every point scored  

---

## 🛠️ Tech & Skills  
- **Platform:** DE1-SoC FPGA  
- **Language:** C (with hardware I/O)  
- **Skills Applied:**  
  - FPGA design  
  - Embedded systems programming  
  - Interrupt-driven input handling  
  - VGA graphics & audio generation  
  - Digital system design  

---

## 🙌 Reflections  
This was such a **rewarding project** that allowed me to blend hardware and software in a hands-on challenge that pushed us to apply everything we learned in **ECE243**. Watching the game come to life on the FPGA was an amazing experience!  

---

## 👩‍💻 Authors  
- **Talaa Abdel**  
- **Tara Sangra**  

