import pymysql

###connect DB

conn = pymysql.connect(host = 'localhost', user= 'db2020', password = 'db2020', db='university')
curs = conn.cursor(pymysql.cursors.DictCursor)

class sugang:

    def menu(self):
        while(1):
            try:
                select = int(input("메뉴 선택( 1~9 , 0 이면 종료): "))
            except:
                select = 10

            if select == 1 :
                self.menu1()
            elif select == 2 :
                self.menu2()
            elif select == 3 :
                self.menu3()
            elif select == 4 :
                self.menu4()
            elif select == 5 :
                self.menu5()
            elif select == 6 :
                self.menu6()
            elif select == 7 :
                self.menu7()
            elif select == 8 :
                self.menu8()
            elif select == 9 :
                self.menu9()
            elif select == 0 :
                break
            else:
                print("다시 입력해 주세요")
    
    def menu1(self):
        print("학생정보를 등록합니다")
        sno=input("학번: ")
        sname=input("이름: ")
        dept=input("학과: ")
        syear=input("학년: ")
        data=(sno,sname,dept,syear)
        sql = "insert into student (sno, sname, dept, syear) values (%s, %s, %s, %s)"
        try:
            curs.execute(sql,data)
            conn.commit()
            print("학생정보 등록완료")
        except:
            print("데이터를 다시 입력해주세요")

    def menu2(self):
        print("학생정보를 삭제합니다")
        sno=input("삭제할 학번: ")
        sql = "delete from student where sno = (%s)"
        sql_check = "select * from student where sno = (%s)" #checking if data exists
        try:
            curs.execute(sql_check,sno)
            if curs.fetchone() is None:
                raise Exception
            curs.execute(sql,sno)
            conn.commit()
            print("학생정보 삭제완료")
        except:
            print("데이터를 다시 입력해주세요")
        
    def menu3(self):
        print("학생정보를 조회합니다")
        sno=input("조회할 학번(전체 조회 키워드: *): ")
        if sno == "*" :
            sql = "select * from student order by sno"
            try:
                curs.execute(sql)
            except:
                print("데이터를 다시 입력해주세요")
        else:
            sql = "select * from student where sno = (%s) order by sno"
            try:
                curs.execute(sql,sno)
            except:
                print("데이터를 다시 입력해주세요")

        row = curs.fetchone()
        while row:
            print("학번 : %d, 이름: %s, 학년: %d, 학과: %s" %(row['sno'],row['sname'],row['syear'],row['dept']) )
            row=curs.fetchone()

    def menu4(self):
        print("과목정보를 등록합니다")
        cno = input("과목 번호: ")
        cname = input("과목 이름: ")
        credit = input("학점: ")
        dept = input("개설학과 : ")
        prname = input("교수님 성함: ")
        data = (cno,cname,credit,dept,prname)
        sql = "insert into course (cno, cname, credit, dept, prname) values (%s, %s, %s, %s, %s)"
        try:
            curs.execute(sql,data)
            conn.commit()
            print("과목정보 등록완료")
        except:
            print("데이터를 다시 입력해주세요")
        
    def menu5(self):
        print("과목정보를 삭제합니다")
        cno=input("삭제할 과목 번호: ")
        sql = "delete from course where cno = (%s)"
        sql_check = "select * from course where cno = (%s)"
        try:
            curs.execute(sql_check,cno)
            if curs.fetchone() is None:
                raise Exception
            curs.execute(sql,cno)
            conn.commit()
            print("과목정보 삭제완료")
        except:
            print("데이터를 다시 입력해주세요")

    def menu6(self):
        print("과목정보를 조회합니다")
        cno=input("조회할 과목 번호(전체 조회 키워드: *): ")
        if cno == "*" :
            sql = "select * from course order by cno"
            curs.execute(sql)
        else:
            sql = "select * from course where cno = (%s) order by cno"
            try:
                curs.execute(sql,cno)
            except:
                print("데이터를 다시 입력해주세요")
        
        row = curs.fetchone()
        while row:
            print("과목 번호 : %s, 과목 이름: %s, 학점: %d, 개설학과: %s, 교수님 성함: %s" %(row['cno'],row['cname'],row['credit'],row['dept'],row['prname']) )
            row=curs.fetchone()
        
    def menu7(self):
        print("수강 신청")
        sno=input("학번: ")
        cno=input("과목 번호: ")
        data=(sno,cno,'C',0,0)
        sql = "insert into enrol (sno, cno, grade, midterm, final) values (%s, %s, %s, %s, %s)"
        try:
            curs.execute(sql,data)
            conn.commit()
            print("신청 완료")
        except:
            print("데이터를 다시 입력해주세요")

    def menu8(self):
        print("수강 취소")
        sno=input("학번: ")
        cno=input("과목 번호: ")
        data=(sno,cno)
        sql = "delete from enrol where sno = (%s) and cno = (%s)"
        sql_check = "select * from enrol where sno = (%s) and cno = (%s)"
        try:
            curs.execute(sql_check,data)
            if curs.fetchone() is None :
                raise Exception
            curs.execute(sql,data)
            conn.commit()
            print("취소 완료")
        except:
            print("데이터를 다시 입력해주세요")

    def menu9(self):
        print("수강 조회")
        try:
            k = int(input("1. 학번 조회 2. 과목 번호 조회 3. 전체 조회: "))
        except:
            k = 0

        if k == 1 :
            data = input("학번 : ")
        elif k == 2 :
            data = input("과목 번호 : ")
        elif k == 3 :
            data = "*"
        else:
            data = None
            print("잘못 입력하였습니다")
        

        if k == 1 and data is not None :
            sql_sname = "select * from student where sno = (%s)"
            try:
                curs.execute(sql_sname , data)
                student_name = curs.fetchone()
                print("%s 의 수강정보" %(student_name['sname']))
            except:
                print("데이터를 다시 입력해주세요")
            
            sql = "select * from enrol where sno = (%s)"
            curs.execute(sql, data)
            
            row = curs.fetchone()
            while row:
                print("과목 번호 : %s, 성적: %s, 중간 성적: %d, 기말 성적: %s" %(row['cno'],row['grade'],row['midterm'],row['final']) )   
                row = curs.fetchone()

        elif k == 2 and data is not None :
            sql_cname = "select * from course where cno = (%s)"
            try:
                curs.execute(sql_cname, data)
                course_name = curs.fetchone()
                print("%s 과목의 수강정보" %(course_name['cname']))
            except:
                print("데이터를 다시 입력해주세요")
            
            sql = "select * from enrol where cno = (%s)"
            curs.execute(sql, data)
            
            row = curs.fetchone()
            while row:
                print("학번 : %s, 성적: %s, 중간 성적: %d, 기말 성적: %s" %(row['sno'],row['grade'],row['midterm'],row['final']) )
                row = curs.fetchone()
        
        elif k == 3 :
            sql = "select * from enrol"
            curs.execute(sql)
            row = curs.fetchone()
            while row:
                print("학번 : %s, 과목 번호 : %s, 성적: %s, 중간 성적: %d, 기말 성적: %s" %(row['sno'],row['cno'],row['grade'],row['midterm'],row['final']) )
                row = curs.fetchone()

### do work now

db=sugang()
db.menu()

