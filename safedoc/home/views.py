# from urllib import request
from django.shortcuts import render,redirect
from django.contrib.auth.models import User
from django.contrib.auth import authenticate,login,logout


def index(respone):
    return render(respone,'index.html')

def dashboard(response):
    return render(response,'dashboard.html')


def register(response):
    if response.method == "POST":
        name = response.POST['name']
        email = response.POST['email']
        password = response.POST['password']
        newuser = User.objects.create_user(name,email,password)
        newuser.save()
        return redirect('/login')
    else:
        return render(response, 'register.html')
    
def ulogin(response):
    if response.method == "POST":
        name = response.POST['name']
        password = response.POST['password']
        user = authenticate(username=name,password=password)
        
        if user is not None:
            print("hello1")
            login(response,user)
            return redirect("home")
        else:
            print("hello2")
            return render(response,'login.html')
        print("Hello3")
    return render(response,'login.html')


def ulogout(response):
    logout(response)
    return redirect("home")