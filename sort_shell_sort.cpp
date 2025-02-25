void shellsort(int *a, int n)
{
    int h;
    for (h = 1; h <= n / 9; h = 3 * h + 1);
    for (; h > 0; h /= 3)
    {
        for (int i = h; i < n; ++i)
        {
            int j = i;
            int tmp = a[i];
            while (j >= h && tmp < a[j - h])
            {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = tmp;
        }
    }
}