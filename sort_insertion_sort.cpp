void insertionsort(int *a, int n)
{
    for (int i = 1; i < n; ++i)
    {
        int tmp = a[i];
        int j = i;
        while (j > 0 && a[j - 1] > tmp)
        {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = tmp;
    }
}