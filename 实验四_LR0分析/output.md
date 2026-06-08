=== 文法分析 ===
开始符号:E 
增广开始符号: E'               
产生式:
0: E' -> E  
1: E -> aA                                       
2: E -> bB                                        
3: A -> cA                                         
4: A -> d                                         
5: B -> cB                                         
6: B -> d
=== LR(0)项目集族 ===
I0:
  E' -> ·E
  E -> ·aA
  E -> ·bB
I1:
  E' -> E·
I2:
  E -> a·A
  A -> ·cA
  A -> ·d
I3:
  E -> b·B
  B -> ·cB
  B -> ·d
I4:
  E -> aA·
I5:
  A -> ·cA
  A -> c·A
  A -> ·d
I6:
  A -> d·
I7:
  E -> bB·
I8:
  B -> ·cB
  B -> c·B
  B -> ·d
I9:
  B -> d·
I10:
  A -> cA·
I11:
  B -> cB·
=== LR(0)分析表 ===
ACTION表:
  状态       a       b       c       d       $
------------------------------------------------
       0      s2      s3                        
       1                                     acc
       2                      s5      s6        
       3                      s8      s9        
       4      r1      r1      r1      r1      r1
       5                      s5      s6        
       6      r4      r4      r4      r4      r4
       7      r2      r2      r2      r2      r2
       8                      s8      s9        
       9      r6      r6      r6      r6      r6
      10      r3      r3      r3      r3      r3
      11      r5      r5      r5      r5      r5
GOTO表:
  状态       A       B       E
--------------------------------
       0                       1
       1                        
       2       4                
       3               7        
       4                        
       5      10                
       6                        
       7                        
       8              11        
       9                        
      10                        
      11                        
