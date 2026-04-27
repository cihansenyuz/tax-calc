# Sık Sorulan Sorular (SSS)

> Bu belge, proje hakkında sık sorulan soruları ve cevaplarını içermektedir.
> Yeni sorular eklemek için aşağıdaki formatı kullanın:
>
> ```
> ## S: Sorunuz buraya
> **C:** Cevap buraya gelecek.
> ```

---

## S: Kapatılan bir pozisyonun gelir vergisine tabii matrahı nasıl hesaplanıyor?

**C:** Vergi matrahı hesabında önce TL cinsinden alım maliyeti ve satım geliri hesaplanır, ardından enflasyon düzeltmesi uygulanır.

**1. Adım — Alım maliyeti ve satım geliri (TL):**

$$\text{Alım Maliyeti} = \text{Kur}_{\text{alim}} \times \bigl(\text{Fiyat}_{\text{alim}} \times \text{Adet} + 1{,}50\bigr)$$

$$\text{Satım Geliri} = \text{Kur}_{\text{satim}} \times \bigl(\text{Fiyat}_{\text{satim}} \times \text{Adet} - 1{,}50\bigr)$$

**2. Adım — Enflasyon düzeltmesi:**

$$\text{Enflasyon Katsayısı} = \frac{\text{TUFE}_{\text{satım}}}{\text{TUFE}_{\text{alım}}}$$

- Katsayı **≥ 1,10** ise enflasyon düzeltmesi uygulanır:

$$\text{Vergi Matrahi} = \text{Satim Geliri} - \bigl(\text{Alim Maliyeti} \times \text{Enflasyon Katsayisi}\bigr)$$

- Katsayı **< 1,10** ise düzeltme yapılmaz:

$$\text{Vergi Matrahi} = \text{Satim Geliri} - \text{Alim Maliyeti}$$

**3. Adım — Negatif matrah koruması:**

Hesaplanan matrah negatif çıkarsa **0** olarak kabul edilir; zarar vergilendirilmez.

---

## S: Yeni pozisyon açtığımda, satın alma işlemine dair komisyon, maliyet hesabına nasıl katılıyor?

**C:** Sabit **1,50 $** aracı kurum komisyonu, alım maliyetine **eklenerek** vergi matrahı hesabına dahil edilir. Formül şu şekildedir:

$$\text{Alım Maliyeti (TL)} = \text{Kur}_{\text{alım}} \times \bigl(\text{Fiyat}_{\text{alım}} \times \text{Adet} + 1{,}50\bigr)$$

`1,50` değeri kur çarpımının **içinde** yer aldığından dolar cinsinden yorumlanır ve alım günündeki dolar kuruyla TL'ye çevrilir. Bu sayede komisyon, toplam maliyeti artırır ve dolayısıyla vergi matrahını düşürür.

---

## S: Mevcut bir pozisyonu seçip, pozisyonu kapattığımda, satma işlemine dair komisyon, maliyet hesabına nasıl katılıyor?

**C:** Sabit **1,50 $** aracı kurum komisyonu, satım gelirinizden **düşülerek** vergi matrahı hesabına dahil edilir. Formül şu şekildedir:

$$\text{Satım Geliri (TL)} = \text{Kur}_{\text{satım}} \times \bigl(\text{Fiyat}_{\text{satım}} \times \text{Adet} - 1{,}50\bigr)$$

`1,50` değeri kur çarpımının **içinde** yer aldığından dolar cinsinden yorumlanır ve satım günündeki dolar kuruyla TL'ye çevrilir. Bu sayede komisyon, net satış gelirinizi azaltır ve dolayısıyla vergi matrahını düşürür.

---

## S: Tablodan çoklu seçim yaptığımda, pozisyonları kapattığımda, satma işlem(ler)ine dair komisyon(lar), maliyet hesabına nasıl katılıyor?

**C:** Çoklu seçimde her pozisyon **bağımsız** olarak işleme alınır. Her bir pozisyon için ayrı ayrı alım komisyonu (+1,50 \$) ve satım komisyonu (−1,50 $) hesaba katılır; her biri kendi işlem günündeki kurla TL'ye çevrilir. Yani N adet pozisyon kapatıyorsanız toplam komisyon etkisi yaklaşık:

$$\text{Toplam Komisyon Etkisi} \approx N \times (1{,}50_{\$,\text{alım}} \times \text{Kur}_{\text{alım}} + 1{,}50_{\$,\text{satım}} \times \text{Kur}_{\text{satım}})$$

Her pozisyonun vergi matrahı ayrı hesaplanıp ardından toplanır.