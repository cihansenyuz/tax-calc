# Sık Sorulan Sorular (SSS)

> Bu belge, proje hakkında sık sorulan soruları ve cevaplarını içermektedir.
> Yeni sorular eklemek için aşağıdaki formatı kullanın:
>
> ```
> ## S: Sorunuz buraya
> **C:** Cevap buraya gelecek.
> ```

---

## S: Yeni pozisyon açtığımda, satın alma işlemine dair komisyon, maliyet hesabına nasıl katılıyor?

**C:** Sabit **1,50 TL** aracı kurum komisyonu, alış maliyetine **eklenerek** vergi matrahı hesabına dahil edilir. Formül şu şekildedir:

$$\text{Alış Maliyeti} = \text{Kur}_{alış} \times \bigl(\text{Fiyat}_{alış} \times \text{Adet} + 1{,}50\bigr)$$

Bu sayede komisyon, toplam maliyeti artırır ve dolayısıyla vergi matrahını düşürür.

---

## S: Mevcut bir pozisyonu seçip, pozisyonu kapattığımda, satma işlemine dair komisyon, maliyet hesabına nasıl katılıyor?

**C:** Sabit **1,50 TL** aracı kurum komisyonu, satış gelirinizden **düşülerek** vergi matrahı hesabına dahil edilir. Formül şu şekildedir:

$$\text{Satış Geliri} = \text{Kur}_{satış} \times \bigl(\text{Fiyat}_{satış} \times \text{Adet} - 1{,}50\bigr)$$

Bu sayede komisyon, net satış gelirinizi azaltır ve dolayısıyla vergi matrahını düşürür.

---

## S: Tablodan çoklu seçim yaptığımda, pozisyonları kapattığımda, satma işlem(ler)ine dair komisyon(lar), maliyet hesabına nasıl katılıyor?

**C:** Çoklu seçimde her pozisyon **bağımsız** olarak işleme alınır. Her bir pozisyon için ayrı ayrı alış komisyonu (+1,50 TL) ve satış komisyonu (−1,50 TL) hesaba katılır. Yani N adet pozisyon kapatıyorsanız toplam komisyon etkisi:

$$\text{Toplam Komisyon Etkisi} = N \times (1{,}50 + 1{,}50) = N \times 3{,}00 \ \text{TL}$$

Her pozisyonun vergi matrahı ayrı hesaplanıp ardından toplanır.