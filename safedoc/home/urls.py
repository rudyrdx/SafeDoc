from django.contrib import admin
from django.urls import path
from home import views


admin.site.site_header = "SafeDoc Admin"
admin.site.site_title = "SafeDoc Admin Portal"
admin.site.index_title = "Welcome to SafeDoc Researcher Portal"


urlpatterns = [
    path('admin/', admin.site.urls),
    path('',views.index,name='home'),
    path('dashboard/',views.dashboard,name='dashboard'),
    path('login/',views.ulogin,name='login'),
    path('logout/',views.ulogout,name='logout'),
    path('register/',views.register,name='register'),
]
