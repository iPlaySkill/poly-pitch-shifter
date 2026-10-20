# Transient Processing and Pick Attack Preservation Specification
# Спецификация обработки транзиентов и сохранения атак медиатора

---

## [RU] Русскоязычная спецификация модуля транзиентной обработки

### 1. Высокочастотный дифференцирующий предкорректирующий фильтр (Pre-Emphasis)
В полифоническом сигнале электрогитары атака медиатора по струнам характеризуется экстремально малым временем нарастания ($\le 1.0$ мс) и высокой концентрацией спектральной энергии в области верхних частот по сравнению со стационарным квазипериодическим гармоническим сустейном колебаний струн. Для надежной селекции атак от низкочастотных основных тонов применяется дискретный дифференцирующий фильтр высоких частот:

$$y[n] = x[n] - 0.95 \cdot x[n-1]$$

#### Частотные характеристики
Передаточная функция фильтра в $z$-плоскости:

$$H(z) = 1 - 0.95 \cdot z^{-1}$$

- На нулевой частоте $\omega = 0$ (постоянная составляющая, низкочастотный резонанс деки): $|H(e^{j0})| = 1 - 0.95 = 0.05$ (ослабление $-26\text{ дБ}$).
- На частоте Найквиста $\omega = \pi$: $|H(e^{j\pi})| = 1 - (-0.95) = 1.95$ (усиление $+5.8\text{ дБ}$).
- Высокочастотные компоненты щелчка медиатора усиливаются более чем на $31\text{ дБ}$ относительно низких гармоник.

```mermaid
graph LR
    X["Входной сигнал x[n]"] --> DIFF["Дифференциатор: y[n] = x[n] - 0.95*x[n-1]"]
    DIFF --> ABS["Модуль энергии: |y[n]|"]
    ABS --> ENV["Асимметричный огибатель"]
    ENV --> ADAPT["Адаптивный динамический порог"]
    ADAPT --> FSM["Конечный автомат (FSM)"]
```

### 2. Асимметричный детектор огибающей и адаптивное пороговое значение
Для мгновенного обнаружения фронта атаки с плавным отслеживанием затухания энергии используется рекурсивный асимметричный огибатель:

$$E[n] = \begin{cases} (1 - \alpha_{\text{att}}) \cdot E[n-1] + \alpha_{\text{att}} \cdot |y[n]|, & \text{если } |y[n]| > E[n-1] \\ (1 - \alpha_{\text{rel}}) \cdot E[n-1] + \alpha_{\text{rel}} \cdot |y[n]|, & \text{если } |y[n]| \le E[n-1] \end{cases}$$

#### Постоянные времени:
- **Время атаки:** $\tau_{\text{att}} = 0.5\text{ мс} \implies \alpha_{\text{att}} \approx 0.80$ при $F_s = 48\text{ кГц}$.
- **Время спада:** $\tau_{\text{rel}} = 15.0\text{ мс} \implies \alpha_{\text{rel}} \approx 0.005$ при $F_s = 48\text{ кГц}$.

#### Адаптивный динамический порог:
Для исключения ложных срабатываний при громком гитарном бое (аккордовом чесе) с сохранением высокой чувствительности в тихих пассажах, порог срабатывания адаптируется к текущему фоновому уровню:

$$B[n] = (1 - \alpha_{\text{bg}}) \cdot B[n-1] + \alpha_{\text{bg}} \cdot E[n]$$

$$\text{Threshold}[n] = \max\left(B[n] \cdot (1.8 \cdot \text{Sensitivity}), 0.002\right)$$

При превышении $E[n] > \text{Threshold}[n]$ детектор инициирует транзиентное событие синхронно для всех каналов стереопары, гарантируя сохранность стереопанорамы и фазовой когерентности.

---

## [ENG] English Transient Processing Specification

### 1. High-Frequency Differentiating Pre-Emphasis Filter
In polyphonic electric guitar audio, string pick strikes exhibit extremely rapid transient risetimes ($\le 1.0$ ms) and concentrated high-frequency spectral content compared to the continuous harmonic sustain of resonating strings. To isolate pick strikes from low-frequency fundamental oscillations, an aggressive discrete-time differentiating pre-emphasis filter is applied:

$$y[n] = x[n] - 0.95 \cdot x[n-1]$$

#### Frequency Response Characteristics
The transfer function of the filter in the $z$-domain is:

$$H(z) = 1 - 0.95 \cdot z^{-1}$$

- At $\omega = 0$ (DC, fundamental guitar body resonance): $|H(e^{j0})| = 1 - 0.95 = 0.05$ ($-26\text{ dB}$ attenuation).
- At $\omega = \pi$ (Nyquist boundary): $|H(e^{j\pi})| = 1 - (-0.95) = 1.95$ ($+5.8\text{ dB}$ boost).
- High frequencies containing the crisp pick click and scratch are boosted by more than $31\text{ dB}$ relative to DC.

```mermaid
graph LR
    X["Input Audio x[n]"] --> DIFF["Differentiator: y[n] = x[n] - 0.95*x[n-1]"]
    DIFF --> ABS["Absolute Energy: |y[n]|"]
    ABS --> ENV["Asymmetric Envelope Follower"]
    ENV --> ADAPT["Adaptive Dynamic Threshold"]
    ADAPT --> FSM["Transient State Machine"]
```

### 2. Asymmetric Envelope Follower & Dynamic Thresholding
To capture the instantaneous onset while smoothly tracking energy decay, an asymmetric recursive envelope follower is employed:

$$E[n] = \begin{cases} (1 - \alpha_{\text{att}}) \cdot E[n-1] + \alpha_{\text{att}} \cdot |y[n]|, & \text{if } |y[n]| > E[n-1] \\ (1 - \alpha_{\text{rel}}) \cdot E[n-1] + \alpha_{\text{rel}} \cdot |y[n]|, & \text{if } |y[n]| \le E[n-1] \end{cases}$$

#### Time Constants
- **Attack time constant:** $\tau_{\text{att}} = 0.5\text{ ms} \implies \alpha_{\text{att}} \approx 0.80$ at $F_s = 48\text{ kHz}$.
- **Release time constant:** $\tau_{\text{rel}} = 15.0\text{ ms} \implies \alpha_{\text{rel}} \approx 0.005$ at $F_s = 48\text{ kHz}$.

#### Adaptive Dynamic Background Threshold
To avoid false triggers during loud continuous guitar strumming while maintaining sensitivity during quiet single-note passages, the trigger threshold adapts dynamically to the running background energy:

$$B[n] = (1 - \alpha_{\text{bg}}) \cdot B[n-1] + \alpha_{\text{bg}} \cdot E[n]$$

$$\text{Threshold}[n] = \max\left(B[n] \cdot (1.8 \cdot \text{Sensitivity}), 0.002\right)$$

When $E[n] > \text{Threshold}[n]$, an attack transient is triggered across stereo channels simultaneously to maintain perfect phase coherence.
